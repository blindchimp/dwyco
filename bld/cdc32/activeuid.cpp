#include "mmchan.h"
#include "activeuid.h"
#include "qauth.h"
#include "qmsgsql.h"
#include "aconn.h"

extern vc Online;
extern vc Chat_ips;
extern vc Client_disposition;
namespace dwyco {

// given a candidate uid, which is expected to be some member
// of a group, decide which member of the group is the most
// likely to be "active". the idea is we want to put a message
// on the device the user might have closest to them.
// note that if this doesn't return the right thing, it isn't a
// disaster, just the message will probably be sent through the
// server or some other path this isn't quite as good.
vc
find_best_candidate_for_initial_send(vc uid, int& skip_direct)
{
    skip_direct = 0;
    vc uids = map_uid_to_uids(uid);
    if(uids.num_elems() <= 1)
        return uid;

    vc last_recv = sql_last_recved_msg(uid);
    // here is where we find the slam-dunks. if we are connected to a
    // "foreground" process and it sent us the latest message, then send it
    // straight back to that uid.
    for(int i = 0; i < last_recv.num_elems(); ++i)
    {
        // note: we know the results are returned with the most recent
        // message first, so once we find it, we're done.
        // WARNING: in weird cases, the time columns returned might
        // be NULL (thanks to sql). if you update this to include
        // checking the times, take that into account.
        vc muid = from_hex(last_recv[i][0]);
        MMChannel *mc = MMChannel::already_connected(muid, 1);
        if(mc && mc->remote_disposition() == vc("foreground"))
        {
            return muid;
        }
    }
    // for now, just keep it simple, and send it via server.
    // this will avoid situations where a message gets sent to
    // a background receiver, only to get "stuck". we might be
    // able to do this in the future when we propagate some
    // notification to others regarding the send, but for now, just
    // let the server broadcast it.
    skip_direct = 1;
    return uid;
#if 0
    //
    // if there are no foreground processes that have sent us anything,
    // we just make an educated guess about where to send the message.
    //
    // we assign a score to various levels of information we
    // have gathered about the uid
    int n = uids.num_elems();
    // this vector isn't strictly necessary since we just want a
    // max, but useful for debugging
    DwSVec<int> scores;
    scores.set_size(n);
    int maxscore = -1;
    int maxi = -1;
    for(int i = 0; i < n; ++i)
    {
        vc u = uids[i];
        int score = 0;
        // note: we can directly connect to some background processing
        // clients.
        if(MMChannel::already_connected(u, 1))
        {
            score += 1;
        }
        if(Chat_ips.contains(u))
            score += 1;
        // uid would time out if it wasn't around, so
        // this is a better candidate
        if(Broadcast_discoveries.contains(u))
            score += 5;
        if(Online.contains(u))
            score += 1;
        vc cd;
        if(Client_disposition.find(u, cd) && cd == vc("foreground"))
            score += 10;

        // if we received a message from them recently
        // give them a higher score.

        if(sql_has_msg_recently(u, 5 * 60))
            score += 5;

        scores.append(score);
        if(score > maxscore)
        {
            maxscore = score;
            maxi = i;
        }
    }
    // note: this is really a programming error, as n >= 1 at this point
    // but naturally, that will get screwed up eventually.
    if(maxi == -1)
        return uid;
    return uids[maxi];
#endif

}


}

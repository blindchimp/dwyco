#include "mmchan.h"
#include "activeuid.h"
#include "qmsg.h"
#include "qmsgsql.h"
#include "aconn.h"

extern vc Online;
extern vc Chat_ips;
namespace dwyco {

// given a candidate uid, which is expected to be some member
// of a group, decide which member of the group is the most
// likely to be "active". the idea is we want to put a message
// on the device the user might have closest to them.
// note that if this doesn't return the right thing, it isn't a
// disaster, just the message will probably be sent through the
// server or some other path this isn't quite as good.
vc
find_best_candidate_for_initial_send(vc uid)
{
    vc uids = map_uid_to_uids(uid);
    if(uids.num_elems() <= 1)
        return uid;

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
        if(MMChannel::already_connected(u, 1))
        {
            // if we are connected to this exact uid, then
            // almost certainly that is where we should send
            if(u == uid)
                score += 1000;
            else
                score += 100;
        }
        if(Chat_ips.contains(u))
            score += 1;
        // uid would time out of if it wasn't around, so
        // this is a better candidate
        if(Broadcast_discoveries.contains(u))
            score += 5;
        if(Online.contains(u))
            score += 1;

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
    return uids[maxi];

}


}

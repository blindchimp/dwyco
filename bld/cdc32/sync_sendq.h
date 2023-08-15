#ifndef SYNC_SENDQ_H
#define SYNC_SENDQ_H
#include "vc.h"
#include "dwtree2.h"

namespace dwyco {

// this implements a simple queue that is used to send sync
// commands to a remote client. the idea is that as you are
// browsing your messages, "pull" requests are generated to other
// clients if you do not have the message locally. the queue also
// accepts "pull-resp" and "idx" messages.
// commands are queued FIFO, with a few twists:
// you can give a priority to the item, and all items with
// a given priority are queued together. this is used in cases where
// a "pull" command is generated through some path that is interactive (ie,
// the user wants to view a message he doesn't have.) in that case,
// the command should be processed before other commands that are
// "background" in nature (like pulling in eager mode.) in addition, the
// response to the pull on the remote side (pull-resp) should be
// higher priority as well. this allows somewhat better response times
// for interactive pulls.
// there are some minor complications: if there is a pull already
// queued with a lower priority, you can increase the priority if it
// becomes the target of an interactive action. NOTE: once a command is
// issued to the network, you can't adjust anything...it is getting sent
// and the response on the remote side will have that lower priority, and
// might get "stuck" behind a bunch of new higher priority requests.
//
// note: this queue is really only exercised in some specific cases...
// if you have a large corpus of messages that you want to sync
// eagerly to a new device that is largely empty, and you need to
// browse the messages while this operation is in progress. once
// the corpus is mostly copied, the size of this queue is tiny, and
// is probably total overkill.
//
class sendq {

    struct qkey {
        qkey() {
            pri = -1;
            clk = -1;
        }
        qkey(int p, int c, vc m, vc cmd) {
            pri = p;
            clk = c;
            mid = m;
            this->cmd = cmd;
        }
        int pri;
        int clk;
        vc mid;
        vc cmd;
        int operator==(const qkey& q) const {
            return pri == q.pri && clk == q.clk;
        }
        int operator<(const qkey& q) const {
            if(pri < q.pri)
                return 1;
            else if(pri > q.pri)
                return 0;
            if(clk < q.clk)
                return 1;
            else if(clk > q.clk)
                return 0;
            return 0;
        }
    };

    DwTreeKaz<int, qkey> q;
    DwTreeKaz<qkey, vc> pull_set;
    static int Clk;

public:
    sendq() : q(0), pull_set(qkey()) {}

    void append(vc cmd, int pri) {
        static vc pull("pull");
        vc mid;
        qkey qk;
        int update = 1;
        if(cmd[0] == pull)
        {
            mid = cmd[1];
            if(pull_set.find(mid, qk))
            {
                // this allows us to increase the priority of
                // some items already in the queue
                if(pri < qk.pri)
                {
                    q.del(qk);
                    cmd = qk.cmd;
                }
                else
                    update = 0;
            }
        }
        if(update)
        {
            qk = qkey(pri, Clk, mid, cmd);
            q.add(qk, 0);
            if(!mid.is_nil())
                pull_set.add(mid, qk);
        }
        Clk++;
    }

    vc delmin() {
        if(q.num_elems() == 0)
            return vcnil;
        qkey qk;
        q.delmin(&qk);
        vc ret = qk.cmd;
        if(!qk.mid.is_nil())
        {
            pull_set.del(qk.mid);
        }
        return ret;
    }

    // this can be used to delete any pulls that are in the
    // q, for example after successfully processing a pull-resp,
    // there is no need to process other  pulls that might be
    // pending to other clients.
    int del_pull(vc mid) {
        qkey qk;
        if(pull_set.find(mid, qk))
        {
            q.del(qk);
            pull_set.del(mid);
            return 1;
        }
        return 0;
    }

    bool contains(vc mid) {
        if(pull_set.contains(mid))
            return true;
        return false;
    }

    int count() const {
        return q.num_elems();
    }
};

}


#endif // SYNC_SENDQ_H

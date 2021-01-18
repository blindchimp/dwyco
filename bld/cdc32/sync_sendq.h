#ifndef SYNC_SENDQ_H
#define SYNC_SENDQ_H
#include "vc.h"
#include "dwtree2.h"

namespace dwyco {

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
                if(qk.pri < pri)
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
};

}


#endif // SYNC_SENDQ_H

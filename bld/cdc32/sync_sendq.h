#ifndef SYNC_SENDQ_H
#define SYNC_SENDQ_H
#include "vc.h"
#include "dwtree2.h"

namespace dwyco {

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


struct sendq {
    sendq() : q(0), pull_set(0) {}

    DwTreeKaz<int, qkey> q;
    DwTreeKaz<int, vc> pull_set;
    static int Clk;

    void append(vc cmd, int pri) {
        static vc pull("pull");
        vc mid;
        if(cmd[0] == pull)
        {
            mid = cmd[1];
            if(pull_set.contains(mid))
                return;
            pull_set.add(mid, 0);
        }
        auto qk = qkey(pri, Clk, mid, cmd);
        q.add(qk, 0);
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

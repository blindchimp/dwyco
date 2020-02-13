#ifndef QLOC_H
#define QLOC_H
#include <QByteArray>
struct QLoc
{
    QByteArray hash;
    QByteArray loc;
    QByteArray lat;
    QByteArray lon;
    QByteArray mid;

    bool operator==(const struct QLoc& ql) const {
        return mid == ql.mid && lat == ql.lat && lon == ql.lon;
    }
};
#endif

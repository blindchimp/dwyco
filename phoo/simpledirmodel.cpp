#include "simpledirmodel.h"
#include "dwycolist2.h"

SimpleDirModel::SimpleDirModel(QObject *parent) :
    QQmlObjectListModel<SimpleDirEntry>(parent, "handle", "uid")
{

}

#define DWYCO_UID "000"
#define DWYCO_HANDLE "001"
#define DWYCO_DESC "002"
#define DWYCO_PREVIEW "005"
#define DWYCO_REVIEWED "006"
#define DWYCO_REGULAR "007"

void
SimpleDirModel::load_from_dwycolist(DWYCO_LIST dl)
{
    static int update_count;
    update_count++;

    dwyco_list qdl(dl);
    dl = 0;

    int n = qdl.rows();
    for(int i = 0; i < n; ++i)
    {
        auto uid = qdl.get<QByteArray>(i, DWYCO_UID);
        auto sde = getByUid(uid);
        if(sde == nullptr)
        {
            sde = new SimpleDirEntry;
            sde->update_uid(uid);
            append(sde);
        }
        int reg = qdl.get_long(i, DWYCO_REVIEWED) && qdl.get_long(i, DWYCO_REGULAR);
        if(1)
        {
        sde->update_handle(qdl.get<QByteArray>(i, DWYCO_HANDLE));
        sde->update_description(qdl.get<QByteArray>(i, DWYCO_DESC));
        sde->update_has_preview(qdl.get_long(i, DWYCO_PREVIEW) == 1);
        }
        else
        {
            sde->update_handle("not regular");
            sde->update_description("not reg");
            sde->update_has_preview(qdl.get_long(i, DWYCO_PREVIEW) == 1);
        }
        sde->update_counter = update_count;
    }
    QList<SimpleDirEntry *> dead;
    for(int i = 0; i < count(); ++i)
    {
        auto sde = at(i);
        if(sde->update_counter < update_count)
            dead.append(sde);
    }
    for(int i = 0; i < dead.count(); ++i)
    {
        remove(dead[i]);
    }
}

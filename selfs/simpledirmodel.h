#ifndef SIMPLEDIRMODEL_H
#define SIMPLEDIRMODEL_H

#include <QObject>
#include "QSortFilterProxyModel"
#include "QQmlObjectListModel.h"
#include "QQmlVarPropertyHelpers.h"
#include "dlli.h"

class SimpleDirEntry : public QObject
{
    friend class SimpleDirModel;

    Q_OBJECT
    QML_READONLY_VAR_PROPERTY(QByteArray, uid)
    QML_READONLY_VAR_PROPERTY(QString, handle)
    QML_READONLY_VAR_PROPERTY(QString, description)
    QML_READONLY_VAR_PROPERTY(bool, has_preview)
    int update_counter;

public:
    SimpleDirEntry(QObject *parent = 0) : QObject(parent) {
        m_has_preview = false;
        update_counter = -1;
    }
};

class SimpleDirModel : public QQmlObjectListModel<SimpleDirEntry>
{
    Q_OBJECT

public:
    explicit SimpleDirModel(QObject *parent = 0);

    void load_from_dwycolist(DWYCO_LIST dl);

};

#endif // SIMPLEDIRMODEL_H


/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QDateTime>
#include <QImage>
#include <QColor>
#include "msglistmodel.h"

static int
dwyco_get_attr(DWYCO_LIST l, int row, const char *col, DwOString& str_out)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        return 0;
    if(type != DWYCO_TYPE_STRING)
        return 0;
    str_out = DwOString(val, 0, len);
    return 1;
}

msglist_model::msglist_model(DWYCO_LIST ddir, QObject *p)
    : QAbstractListModel(p)
{
    msg_idx = ddir;
}

msglist_model::~msglist_model()
{
    dwyco_list_release(msg_idx);
}

int
msglist_model::rowCount ( const QModelIndex & parent) const
{
    if(parent.isValid())
        return 0;
    if(!msg_idx)
        return 0;
    int rows;
    dwyco_list_numelems(msg_idx, &rows, 0);
    return rows;
}


QVariant
msglist_model::data ( const QModelIndex & index, int role ) const
{
    if(!index.isValid())
        return QVariant();
    // col 0 = name
    // col 1 = location
    // col 2 = desc

    const char *out;
    int len_out, type_out;
    const char *ourcol;
    switch(index.column())
    {
    case 0:
        //ourcol = DWYCO_QM_BODY_DATE_SECONDS_SINCE_JAN1_1970;
        ourcol = DWYCO_MSG_IDX_DATE;
        break;
    //case 1: ourcol = DWYCO_DIR_INFO_LOCATION; break;
    //case 2: ourcol = DWYCO_DIR_INFO_DESC; break;
    default:
        return QVariant();
    }
    if(role == Qt::UserRole)
    {
        DwOString mid;
        if(!dwyco_get_attr(msg_idx, index.row(), DWYCO_MSG_IDX_MID, mid))
            return QVariant();
        QByteArray a(mid.c_str(), mid.length());
        return QVariant(a);
    }
    if(role == Qt::DisplayRole)
    {
        if(!dwyco_list_get(msg_idx, index.row(), ourcol,
                           &out, &len_out, &type_out))
            return QVariant();
        if(type_out != DWYCO_TYPE_INT)
            return QVariant();
        //return QVariant(QString(out));
        QDateTime q(QDateTime::fromSecsSinceEpoch(atol(out)));
        return QVariant(q);
    }
    else if(role == Qt::ForegroundRole)
    {
        DwOString str;
        if(dwyco_get_attr(msg_idx, index.row(), DWYCO_MSG_IDX_IS_SENT, str))
        {
            return QVariant(QColor(Qt::green));
        }
        return QVariant();
    }
    else if( role == Qt::DecorationRole)
    {
        DwOString str;
        DwOString mid;
        if(!dwyco_get_attr(msg_idx, index.row(), DWYCO_MSG_IDX_MID, mid))
            return QVariant();
        if(dwyco_mid_has_tag(mid.c_str(), "_trash"))
        {
            return QVariant(QImage(":/new/prefix1/kill.png"));
        }

        if(dwyco_get_attr(msg_idx, index.row(), DWYCO_MSG_IDX_HAS_ATTACHMENT, str) == 0)
        {
            return QVariant();
        }
        if(dwyco_get_attr(msg_idx, index.row(), DWYCO_MSG_IDX_IS_FILE, str) != 0)
        {
            return QVariant(QImage(":/new/prefix1/downlaod-16x16.png"));
        }
        if(dwyco_get_attr(msg_idx, index.row(), DWYCO_MSG_IDX_ATT_HAS_VIDEO, str) != 0)
        {
            if(dwyco_get_attr(msg_idx, index.row(), DWYCO_MSG_IDX_ATT_IS_SHORT_VIDEO, str) != 0)
                return QVariant(QImage(":/new/prefix1/media 1-16x16.png"));
            else
                return QVariant(QImage(":/new/prefix1/media 2-16x16.png"));
        }
        if(dwyco_get_attr(msg_idx, index.row(), DWYCO_MSG_IDX_ATT_HAS_AUDIO, str) != 0)
            return QVariant(QImage(":/new/prefix1/music 2-16x16.png"));
        return QVariant(QImage(":/new/prefix1/stop.png").scaled(QSize(16, 16)));
    }
    return QVariant();

}

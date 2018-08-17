
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "lobbybox3.h"
#include "ui_lobbybox3.h"
#include "mainwin.h"
#include "dlli.h"

extern DWYCO_LIST Dwyco_server_list;
int set_current_server(int i);
int set_current_server(const DwOString&);
extern DwOString Current_server_id;
extern DwOString Last_selected_id;

lobbybox3::lobbybox3(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::lobbybox3)
{
    ui->setupUi(this);

    int n;
    int cols;
    dwyco_list_numelems(Dwyco_server_list, &n, &cols);
    for(int i = 0; i < n; ++i)
    {
        const char *out;
        int out_len;
        int out_type;
        if(dwyco_list_get(Dwyco_server_list, i, DWYCO_SL_SERVER_NAME,
                          &out, &out_len, &out_type) == 0)
            break;
        if(out_type != DWYCO_TYPE_STRING)
            break;

        QListWidgetItem *twi = new QListWidgetItem;
        twi->setText(out);
        twi->setData(Qt::UserRole + 1, i);
        ui->tw->addItem(twi);
    }

    connect(Mainwinform, SIGNAL(add_user_lobby(user_lobby)),
            this, SLOT(add_lobby(user_lobby)));
    connect(Mainwinform, SIGNAL(del_user_lobby(DwOString)),
            this, SLOT(del_lobby(DwOString)));
    connect(Mainwinform, SIGNAL(clear_user_lobbies()),
            this, SLOT(clear_lobbies()));

}

lobbybox3::~lobbybox3()
{
    delete ui;
}

void
lobbybox3::add_lobby(user_lobby ul)
{
    // don't add it again if it is already in there

    int n = ui->tw->count();
    for(int i = 0; i < n; ++i)
    {
        QListWidgetItem *qtwi = ui->tw->item(i);

        QVariant q;
        q = qtwi->data(Qt::UserRole);
        if(q != QVariant::Invalid)
        {
            if(ul.id.eq(q.toList()[0].toString().toAscii().constData()))
            {
                return;
            }
        }
    }

    QList<QVariant> qvl;
    QVariant q(QString(ul.id.c_str()));
    qvl.append(q);
    q = QString(ul.category.c_str());
    qvl.append(q);

    QListWidgetItem *qtwi = new QListWidgetItem;
    qtwi->setText(ul.dispname.c_str());
    qtwi->setData(Qt::UserRole, qvl);
    ui->tw->addItem(qtwi);
    if(ul.id == Last_selected_id)
    {
        qtwi->setSelected(1);
    }
}

void
lobbybox3::del_lobby(DwOString id)
{
    int n = ui->tw->count();

    int i;
    for(i = 0; i < n; ++i)
    {
        QListWidgetItem *qtwi = ui->tw->item(i);

        QVariant q;
        q = qtwi->data(Qt::UserRole);
        if(q != QVariant::Invalid)
        {
            if(id.eq(q.toList()[0].toString().toAscii().constData()))
            {
                break;
            }
        }
    }
    if(i == n)
        return;
    // if we are logged into this lobby, we need to sever all our
    // connections to it (eventually)
    QListWidgetItem *qi = ui->tw->item(i);
    delete qi;
}

void
lobbybox3::clear_lobbies()
{
    // clear just user lobbies
    int n = ui->tw->count();
    QList<QListWidgetItem *> del_list;
    for(int i = 0; i < n; ++i)
    {
        QListWidgetItem *qtwi = ui->tw->item(i);

        QVariant q;
        q = qtwi->data(Qt::UserRole);
        if(q != QVariant::Invalid)
        {
            ui->tw->takeItem(i);
            delete qtwi;
            --i;
            --n;
        }
    }

}

void
lobbybox3::on_tw_itemDoubleClicked(QListWidgetItem *item)
{

    QVariant sl = item->data(Qt::UserRole + 1);
    if(sl.isValid())
    {
        if(!set_current_server(sl.toInt()))
            return;
    }
    else
    {

        QVariant q;
        q = item->data(Qt::UserRole);
        if(q == QVariant::Invalid)
            return;
        QString id = q.toList()[0].toString();

        if(!set_current_server(id.toAscii().constData()))
            return;
    }

}

void
lobbybox3::on_tw_itemClicked(QListWidgetItem *item)
{
    on_tw_itemDoubleClicked(item);
}


void
lobbybox3::on_tw_itemSelectionChanged()
{
    //ui.user_lobby_list->clearSelection();
}



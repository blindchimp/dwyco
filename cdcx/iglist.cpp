
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QtGui>
#include <QDesktopServices>
#include <QMessageBox>
#include "dwycolistscoped.h"
#include "ui_iglist.h"
#include "iglist.h"
#include "dwstr.h"
#include "ssmap.h"
#include "dlli.h"
#include "tfhex.h"
#include "mainwin.h"
#include "qval.h"

iglistform *TheIglistForm;

iglistform::iglistform(QDialog *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    refresh();
    connect(this, SIGNAL(ignore_event(DwOString)), Mainwinform, SIGNAL(ignore_event(DwOString)));
    setAttribute(Qt::WA_QuitOnClose, 0);
}


void
iglistform::accept()
{
    QDialog::accept();
}

void
iglistform::on_refresh_button_clicked()
{
    refresh();
}

void
iglistform::on_unblock_button_clicked()
{
    QList<QListWidgetItem *> sel;
    sel = ui.list->selectedItems();
    int n = sel.count();
    if(n == 0)
        return;
    for(int i = 0; i < n; ++i)
    {
        QListWidgetItem *it = sel[i];
        QVariant quid = it->data(Qt::UserRole);
        QByteArray buid(quid.toByteArray());
        dwyco_unignore(buid.constData(), buid.length());
        emit ignore_event(DwOString(buid.constData(), 0, buid.length()));
    }
    refresh();
}

void
iglistform::refresh()
{
    ui.list->clear();

    DWYCO_LIST _il = dwyco_ignore_list_get();
    simple_scoped il(_il);
    int n = il.rows();

    for(int i = 0; i < n; ++i)
    {
        const char *uid;
        int len_uid;
        int type;
        if(!dwyco_list_get(il, i, DWYCO_NO_COLUMN,
                           &uid, &len_uid, &type))
            continue;
        if(type != DWYCO_TYPE_STRING)
            continue;

        DwOString duid(uid, 0, len_uid);
        QVariant quid(QByteArray(uid, len_uid));
        QString val(dwyco_info_to_display(duid));
        val.truncate(25);
        val += " (#";
        DwOString hex(to_hex(duid));
        hex.remove(8);
        val += hex.c_str();
        val += ")";
        QListWidgetItem *ni = new QListWidgetItem(val, ui.list);
        ni->setData(Qt::UserRole, quid);
    }
}


/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef MSGBROWSEDOCK_H
#define MSGBROWSEDOCK_H

#include <QDockWidget>
#include <QMenu>
#include <QList>
#include <QString>
#include <QItemSelection>
#include "dwstr.h"
#include "dlli.h"
#include "msglistmodel.h"

namespace Ui {
class MsgBrowseDock;
}

class MsgBrowseDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit MsgBrowseDock(QWidget *parent = 0);
    ~MsgBrowseDock();

    DwOString uid;
    msglist_model *mm;
    QMenu *popup_menu;
    int vis;

    int load_model(DwOString uid, int init = 1);
    void reset();
    QList<DwOString> *get_selection(QModelIndexList* &, int only_one);

    void contextMenuEvent(QContextMenuEvent *ev);

protected slots:
    void on_actionDelete_triggered(bool);
    void on_actionForward_triggered(bool);
    void on_actionReply_triggered(bool);
    void on_actionClose_triggered(bool);

private slots:
    void cur_change(const QModelIndex& idx, const QModelIndex& prev);
    void deferred_load(bool);

    void on_actionDelete_All_triggered();
    void uid_selected_event(DwOString, int);

    void on_show_sent_clicked(bool checked);
    void vis_change(bool);

    void process_ignore_event(DwOString);
    void track_multiselect(QItemSelection, QItemSelection);

private:
    Ui::MsgBrowseDock *ui;
};

#endif // MSGBROWSEDOCK_H

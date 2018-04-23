
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef LOBBYBOX3_H
#define LOBBYBOX3_H

#include <QDockWidget>
#include <QListWidgetItem>
#include "dwstr.h"
#include "userlob.h"

namespace Ui {
class lobbybox3;
}

class lobbybox3 : public QDockWidget
{
    Q_OBJECT

public:
    explicit lobbybox3(QWidget *parent = 0);
    ~lobbybox3();


public slots:
    void add_lobby(user_lobby ul);
    void del_lobby(DwOString);
    void clear_lobbies();
    void on_tw_itemDoubleClicked(QListWidgetItem *);
    void on_tw_itemClicked(QListWidgetItem *);
    void on_tw_itemSelectionChanged();
private:
    Ui::lobbybox3 *ui;
};

#endif // LOBBYBOX3_H

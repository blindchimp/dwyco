
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef autoupdate_h
#define autoupdate_h

#include <QWidget>
#include <QDialog>
#include "ui_autoupdate.h"

class autoupdateform : public QDialog
{
    Q_OBJECT

public:
    autoupdateform(QDialog *parent = 0, Qt::WindowFlags f = 0);
    ~autoupdateform();
    int forced;
    int inhibit_unforced;

private slots:
    void on_download_button_clicked();
    void on_cancelButton_clicked();
    void on_shutdown_and_update_button_clicked();

    void on_textBrowser_anchorClicked(const QUrl &arg1);

public:
    Ui::autoupdate_dialog ui;
};
#endif


/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef SIMPLEHTMLBOX_H
#define SIMPLEHTMLBOX_H

#include <QDockWidget>
#include <QNetworkReply>

namespace Ui {
class SimpleHtmlBox;
}

class SimpleHtmlBox : public QDockWidget
{
    Q_OBJECT

public:
    explicit SimpleHtmlBox(QWidget *parent = 0);
    ~SimpleHtmlBox();

    int default_url;
    QUrl url;
    QNetworkReply *reply;
    virtual void create_new_url() = 0;
    void set_to_default_url();


public slots:
    void replyFinished(QNetworkReply *);
    void reload_triggered();
    void reply_finished();

private slots:
    void on_filter_clicked();

protected:
    Ui::SimpleHtmlBox *ui;
};

#endif // SIMPLEHTMLBOX_H

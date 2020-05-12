
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef BROWSEBOX_H
#define BROWSEBOX_H
#ifdef CDCX_WEBKIT
#include <QDockWidget>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QWebEnginePage>
#include "dwstr.h"
#include <QMenu>


namespace Ui {
class BrowseBox;
}

class BrowseBox : public QDockWidget
{
    Q_OBJECT

public:
    explicit BrowseBox(QWidget *parent = 0);
    ~BrowseBox();

    QMenu *popup_menu;
    DwOString context_uid;
    int default_url;
    QUrl url;
    QUrl hovered_url;

    void set_url(QUrl& url);
    void set_to_default_url();
    virtual void create_new_url() = 0;

public slots:
    void link_clicked(const QUrl&);
    void reload_triggered();
    void hover(const QString &);

    void on_actionView_profile_ctx_triggered();
    void on_actionIgnore_user_ctx_triggered();
    void on_actionCompose_msg_ctx_triggered();
    void on_actionShow_Chatbox_ctx_triggered();
    void on_actionSend_file_ctx_triggered();

    void on_filter_clicked();
    void sslErrorHandler(QNetworkReply*, const QList<QSslError> &);

signals:
    void ignore_event(DwOString);
    void uid_selected(DwOString, int);
    void local_channel_send(DwOString, int, bool);

protected:
    Ui::BrowseBox *ui;
};

class DelPage : public QWebEnginePage
{
    Q_OBJECT

public:
    DelPage(QObject *o = 0) : QWebEnginePage(o) {}


    virtual bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame);
signals:
    void link_clicked(const QUrl&);

};

#endif

#endif // BROWSEBOX_H

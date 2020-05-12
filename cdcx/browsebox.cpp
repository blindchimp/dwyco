
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef CDCX_WEBKIT
#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>
#include <QSslError>
#include "browsebox.h"
#include "ui_browsebox.h"
#include "mainwin.h"
#include "tfhex.h"
#include "composer.h"
#include "dlli.h"
#include "simple_call.h"

bool
DelPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    QUrl q = url;

    if(q.scheme() == "cdcx" && type == QWebEnginePage::NavigationTypeLinkClicked)
    {
        DwOString uid = from_hex(q.path().toAscii().constData());
        emit link_clicked(url);
        return false;
    }
    return true;
}

BrowseBox::BrowseBox(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::BrowseBox)
{
    ui->setupUi(this);
    default_url = 1;

    DelPage *p = new DelPage;
    ui->webView->setPage(p);

    connect(p, SIGNAL(link_clicked(const QUrl&)), this, SLOT(link_clicked(const QUrl&)));

    QToolBar *tb = new QToolBar(ui->dockWidgetContents);
    QAction *qa = ui->webView->pageAction(QWebEnginePage::Reload);
    qa->setIcon(QIcon(":new/red32/icons/PrimaryCons_Red_KenSaunders/PNGs/32x32/refresh-32x32.png"));
    tb->addAction(qa);
    connect(qa, SIGNAL(triggered()), this, SLOT(reload_triggered()));
    ui->verticalLayout->insertWidget(0, tb);

    //ui->webView->page()->setLinkDelegationPolicy(QWebEnginePage::DelegateAllLinks);
    //QNetworkAccessManager *nam = ui->webView->page()->networkAccessManager();
    //connect(nam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslErrorHandler(QNetworkReply*,QList<QSslError>)));
    //connect(ui->webView->page(), SIGNAL(linkClicked(const QUrl&)), SIGNAL(linkClicked(const QUrl&)));
    //connect(ui->webView, SIGNAL(linkClicked(QUrl)), this, SLOT(link_clicked(QUrl)));
    connect(this, SIGNAL(uid_selected(DwOString, int)), Mainwinform, SIGNAL(uid_selected(DwOString,int)));
    connect(this, SIGNAL(ignore_event(DwOString)), Mainwinform, SIGNAL(ignore_event(DwOString)));
    //QAction *a = ui->webView->pageAction(QWebEnginePage::OpenLinkInThisWindow);
    popup_menu = new QMenu(this);
    popup_menu->addAction(ui->actionView_profile_ctx);
    popup_menu->addAction(ui->actionShow_Chatbox_ctx);
    popup_menu->addAction(ui->actionCompose_msg_ctx);
    popup_menu->addAction(ui->actionSend_file_ctx);
    popup_menu->addAction(ui->actionIgnore_user_ctx);
    //connect(ui->webView->page(), SIGNAL(linkHovered(const QString&)), this, SLOT(hover(const QString&)));
    //connect(a, SIGNAL(triggered()), this, SLOT(link_clicked()));

}

BrowseBox::~BrowseBox()
{
    delete ui;
}

void
BrowseBox::hover(const QString &u)
{
    hovered_url = u;
}

void
BrowseBox::sslErrorHandler(QNetworkReply* qnr, const QList<QSslError> & sslErrs)
{
#ifndef CDCX_NO_SSL
    qDebug() << "handleSslErrors: ";
    foreach (QSslError e, sslErrs)
    {
        qDebug() << "ssl error: " << e;
    }
    qnr->ignoreSslErrors(sslErrs);
#endif
}

void
BrowseBox::set_url(QUrl &url)
{
    //QWebSettings::clearMemoryCaches();
    ui->webView->setUrl(url);
    default_url = 0;
}

void
BrowseBox::set_to_default_url()
{
    //QWebSettings::clearMemoryCaches();
    ui->webView->setUrl(QUrl("http://www.dwyco.com"));
    default_url = 1;
}

void
BrowseBox::reload_triggered()
{
//    if(default_url)
    // {
    create_new_url();
    set_url(url);
//    }

}

void
BrowseBox::link_clicked(const QUrl& q)
{
    if(q.scheme() == "cdcx")
    {
        DwOString uid = from_hex(q.path().toAscii().constData());
        emit uid_selected(uid, 1);
        popup_menu->popup(QCursor::pos());
        context_uid = uid;
        dwyco_field_debug("link-click", 1);
    }
    else
    {
        QDesktopServices::openUrl(q);
    }

}

void BrowseBox::on_actionShow_Chatbox_ctx_triggered()
{
    simple_call *c = simple_call::get_simple_call(context_uid);

    c->show();
    c->raise();
    Mainwinform->organize_chatboxes(c);
    dwyco_field_debug("browse-chatbox", 1);
}

void BrowseBox::on_actionCompose_msg_ctx_triggered()
{
    composer *c = new composer(COMP_STYLE_REGULAR, 0, this);
    c->set_uid(context_uid);
    c->show();
    c->raise();
    dwyco_field_debug("browse-composer", 1);
}

void BrowseBox::on_actionIgnore_user_ctx_triggered()
{
    if(confirm_ignore())
        dwyco_ignore(context_uid.c_str(), context_uid.length());
    emit ignore_event(context_uid);
}

void BrowseBox::on_actionView_profile_ctx_triggered()
{
    viewer_profile *c;

    DwOString duid = context_uid;
    if(!Allow_multiple_windows)
        viewer_profile::delete_all();
    c = viewer_profile::get_viewer_profile(duid);
    if(!c)
    {
        c = new viewer_profile(this);
        c->set_uid(duid);
        c->show();
        c->start_fetch();
    }
    else
    {
        c->show();
        c->raise();
        c->showNormal();
    }
    dwyco_field_debug("browse-profile", 1);

}



void BrowseBox::on_actionSend_file_ctx_triggered()
{
    DwOString uid = context_uid;
    if(uid.length() == 0)
        return;
    send_file_common(uid);
}

void
BrowseBox::on_filter_clicked()
{
    reload_triggered();
    dwyco_field_debug("browse-filter", 1);
}
#endif

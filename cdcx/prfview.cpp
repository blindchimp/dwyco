
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "prfview.h"
#include "ui_prfview.h"
#include "dlli.h"
#include "mainwin.h"


DwQueryByMember<PrfView> PrfView::Prf_views;

PrfView::PrfView(const DwOString &uid, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrfView),
    vp(this)
{
    ui->setupUi(this);
    connect(Mainwinform, SIGNAL(invalidate_profile(DwOString)), this, SLOT(update_profile(DwOString)));
    connect(ui->label, SIGNAL(mouse_release()), ui->actionPlay, SLOT(trigger()));
    //connect(ui->label, SIGNAL(mouse_release()), ui->actionStop, SLOT(trigger()));
    ui->actionPlay->setEnabled(0);
    ui->actionStop->setEnabled(0);
    viewid = -1;
    reviewed = 0;
    regular = 0;
    this->uid = uid;

    setAttribute(Qt::WA_QuitOnClose, 0);
    setAttribute(Qt::WA_DeleteOnClose);

    Prf_views.add(this);
}

PrfView::~PrfView()
{
    vp.invalidate();
    dwyco_delete_zap_view(viewid);
    Prf_views.del(this);
    delete ui;
}
bool
PrfView::event(QEvent *event)
{
    return QWidget::event(event);
}

static void
DWYCOCALLCONV
dwyco_play_done(int /*id*/, void *arg)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    PrfView *c = (PrfView *)(void *)vp;
    c->ui->actionStop->setEnabled(0);
    c->ui->actionPlay->setEnabled(1);
}



static
void
DWYCOCALLCONV
dwyco_profile_fetch_done(
    int succ, const char *reason,
    const char *s1, int len_s1,
    const char *s2, int len_s2,
    const char *s3, int len_s3,
    const char *filename,
    const char *uid, int len_uid,
    int reviewed, int regular,
    void *arg)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    PrfView *c = (PrfView *)(void *)vp;
    c->fetch_done(
        succ, reason,
        s1, len_s1,
        s2, len_s2,
        s3, len_s3,
        filename,
        uid, len_uid,
        reviewed, regular
    );
}

void
PrfView::fetch_done(
    int succ, const char *reason,
    const char *s1, int len_s1,
    const char *s2, int len_s2,
    const char *s3, int len_s3,
    const char *filename,
    const char *uid, int len_uid,
    int reviewed, int regular)

{
    if(succ == 0)
    {
        ui->textBrowser->setText(QString("No profile available. (%1)").arg(reason));
        ui->label->hide();
        return;
    }

    dwyco_info_to_display(uid, handle, desc, loc, email);

    ui->textBrowser->setText(handle + QString("\n") + desc);
    if(succ == -1)
    {
        ui->label->clear();
        ui->label->setVisible(0);
        ui->actionPlay->setEnabled(0);
        ui->actionStop->setEnabled(0);

    }
    else
    {
        ui->label->setVisible(1);
        if(viewid != -1)
            dwyco_delete_zap_view(viewid);
        viewid = succ;
        int video = 0;
        int audio = 0;
        int short_video = 0;
        if(dwyco_zap_quick_stats_view(viewid, &video, &audio, &short_video))
        {
            // in cdc-x, pic-only profiles don't happen at this point.
#if 0
            if(short_video)
            {
                ui->actionPlay->setEnabled(0);
                ui->actionStop->setEnabled(0);
                ui->statusbar->showMessage("Pic only");
            }
            else
#endif
                if(video && !audio)
                {
                    ui->actionPlay->setEnabled(1);
                    ui->actionStop->setEnabled(0);
                    //ui->statusbar->showMessage("Video only (no audio)");
                }
                else if(!video && audio)
                {
                    ui->actionPlay->setEnabled(1);
                    ui->actionStop->setEnabled(0);
                    //ui->statusbar->showMessage("Audio only (no video)");
                    ui->label->clear();
                    ui->label->setText("(audio only)");
                }
                else if(!audio && !video)
                {
                    ui->actionPlay->setEnabled(0);
                    ui->actionStop->setEnabled(0);
                    //ui->statusbar->showMessage("Text only");
                    ui->label->clear();
                    ui->label->setVisible(0);
                }
                else
                {
                    ui->actionPlay->setEnabled(1);
                    ui->actionStop->setEnabled(0);
                    //ui->statusbar->showMessage("Audio+Video");
                }
        }
        dwyco_zap_play_preview(viewid, 0, 0, &ui->label->ui_id);

    }
}

void
PrfView::update_profile(DwOString uid)
{
    if(uid == this->uid)
        start_fetch();
}

void
PrfView::closeEvent(QCloseEvent *ev)
{
    dwyco_delete_zap_view(viewid);
    viewid = -1;
    ui->actionPlay->setEnabled(0);
    ui->actionStop->setEnabled(0);
    QWidget::closeEvent(ev);
}

int
PrfView::start_fetch()
{
    ui->textBrowser->setText("Fetching profile...");
    ui->label->clear();
    ui->actionPlay->setEnabled(0);
    ui->actionStop->setEnabled(0);
    QString a("Profile: ");
    a += dwyco_info_to_display(uid);
    setWindowTitle(a);
    return dwyco_get_profile_to_viewer(uid.c_str(), uid.length(), dwyco_profile_fetch_done, (void *)vp.cookie);
}

#if 0
void
PrfView::on_actionPlay_triggered(bool)
{
    if(!dwyco_zap_play_view(viewid, dwyco_play_done, (void *)vp.cookie, &ui_id))
        return;
    ui->label->ui_id = ui_id;
    ui->actionStop->setEnabled(1);
    ui->actionPlay->setEnabled(0);
}

void
PrfView::on_actionStop_triggered(bool)
{
    if(!dwyco_zap_stop_view(viewid))
        return;
    ui->actionStop->setEnabled(0);
    ui->actionPlay->setEnabled(1);
}
#endif

void PrfView::on_actionPlay_triggered()
{
    if(!ui->actionPlay->isEnabled())
    {
        if(!dwyco_zap_stop_view(viewid))
            return;
        ui->actionPlay->setEnabled(1);
        return;
    }
    if(!dwyco_zap_play_view(viewid, dwyco_play_done, (void *)vp.cookie, &ui->label->ui_id))
        return;

    ui->actionPlay->setEnabled(0);
}



void PrfView::on_actionStop_triggered()
{
    if(!ui->actionStop->isEnabled())
        return;
    if(!dwyco_zap_stop_view(viewid))
        return;
    ui->actionStop->setEnabled(0);
    ui->actionPlay->setEnabled(1);
}

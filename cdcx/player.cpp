
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include "player.h"
#include "dlli.h"
#include "composer.h"
#include "dwycolistscoped.h"

QList<DVP> player::Players;
DwOString dwyco_get_attr(DWYCO_LIST l, int row, const char *col);

player::player(QWidget *parent, Qt::WindowFlags w) :
    QWidget(parent, w), vp(this)
{
    ui.setupUi(this);
    ui.stop_button->setEnabled(0);
    ui.play_button->setEnabled(0);
    ui.stop_button->hide();
    ui.play_button->hide();
    viewid = 0;
    ui_id = -1;
    Players.append(vp);
}

player::~player()
{
    int i;
    if((i = Players.indexOf(vp)) == -1)
        cdcxpanic("bad player");
    Players.removeAt(i);
    vp.invalidate();
    dwyco_zap_stop_view(viewid);
    dwyco_delete_zap_view(viewid);
}

void
player::reset()
{
    on_stop_button_clicked();
    ui.stop_button->setEnabled(0);
    ui.play_button->setEnabled(0);
    ui.stop_button->hide();
    ui.play_button->hide();
    ui.label->clear();
    ui.label->setText("...select user to see saved msgs -->");
    viewid = 0;
    ui_id = -1;
}

#if 0
void
player::resizeEvent(QResizeEvent *re)
{
    ui.label->resizeEvent(re);
    QWidget::resizeEvent(re);
}
#endif

static
void
DWYCOCALLCONV
preview_done(int id, void *arg)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    player *c = (player *)(void *)vp;

    int video;
    int audio;
    int short_video;
    if(!dwyco_zap_quick_stats_view(c->viewid, &video, &audio, &short_video))
    {
        c->ui.play_button->hide();
        c->ui.stop_button->hide();
        return;
    }
    if((video || audio) && !short_video)
    {
        c->ui.play_button->show();
        c->ui.stop_button->show();
        c->ui.play_button->setEnabled(1);
        c->ui.stop_button->setEnabled(0);
        if(audio && !video)
        {
            c->ui.label->clear();
            c->ui.label->setText("(audio only)");
        }
    }
    else
    {
        c->ui.play_button->hide();
        c->ui.stop_button->hide();
    }
}

int
player::preview_saved_msg(DwOString uid, DwOString mid)
{
    if(viewid)
    {
        dwyco_delete_zap_view(viewid);
        viewid = 0;
    }
    filename = "";

    this->uid = uid;
    this->mid = mid;

    DWYCO_SAVED_MSG_LIST qsm;
    if(!dwyco_get_saved_message(&qsm, uid.c_str(), uid.length(), mid.c_str()))
    {
        return 0;
    }
    simple_scoped sm(qsm);
    DwOString fa = dwyco_get_attr(sm, 0, DWYCO_QM_BODY_FILE_ATTACHMENT);
    if(fa.length() > 0)
    {
        // file attachment, show the "save it" "dialog"
        filename = fa;
        ui.stop_button->hide();
        ui.play_button->setText("Save...");
        QString txt("File attached: \"");
        txt += fa.c_str();
        txt += "\"";
        ui.label->setPixmap(QPixmap());
        ui.label->setText(txt);
        ui.play_button->setEnabled(1);
        ui.play_button->show();
        ui.label->show();
        preview_file_zap(sm, ui.label, fa, uid, mid);
    }
    else
    {
        // this size constraint stuff needs to be fixed... previewing a file zap
        // should allow resizing instead of fixing the size to 320x240. in this
        // branch, we undo the contraint.
        ui.label->setFixedSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        ui.play_button->setText("");
        viewid = dwyco_make_zap_view2(sm, 0);
        if(viewid == 0)
            return 0;
        if(!dwyco_zap_play_preview(viewid, preview_done, (void *)vp.cookie, &ui_id))
        {
            dwyco_delete_zap_view(viewid);
            viewid = 0;
            return 0;
        }
        ui.label->ui_id = ui_id;
    }
    return 1;
}

static
void
DWYCOCALLCONV
stop_view(int id, void *arg)
{
    DVP vp = DVP::cookie_to_ptr((DVP_COOKIE)arg);
    if(!vp.is_valid())
        return;
    player *c = (player *)(void *)vp;

    c->ui.play_button->setEnabled(1);
    c->ui.stop_button->setEnabled(0);
}

void
player::on_play_button_clicked()
{
    if(filename.length() > 0)
    {
        do_file_save();
        return;
    }
    if(!viewid)
        return;
    if(!dwyco_zap_play_view(viewid, stop_view, (void *)vp.cookie, &ui_id))
        return;
    ui.label->ui_id = ui_id;
    ui.play_button->setEnabled(0);
    ui.stop_button->setEnabled(1);
}

void
player::on_stop_button_clicked()
{
    if(!viewid)
        return;
    if(!dwyco_zap_stop_view(viewid))
        return;
    ui.play_button->setEnabled(1);
    ui.stop_button->setEnabled(0);
}

void
player::do_file_save()
{
    QString tfn(filename.c_str());
    QString dt = QDir::home().absoluteFilePath(tfn);

    //QString fn = QFileDialog::getSaveFileName(this, "Save File", dt, QString(), 0, QFileDialog::DontUseNativeDialog);

#if (!defined(LINUX) && !defined(MAC_CLIENT))
// windows wants to change the cwd on us, so tell it to
// use the qt dialog.
    // apparently despite our request otherwise, qt decides to use the native dialog
    //QString fn = QFileDialog::getOpenFileName(0, "File to send", QString(), QString(), 0, QFileDialog::DontUseNativeDialog);

    QFileDialog fd;
    fd.setOption(QFileDialog::DontUseNativeDialog);
    QString docs = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if(!docs.isEmpty())
        fd.setDirectory(docs);
    fd.selectFile(tfn);
    fd.setFileMode(QFileDialog::AnyFile);
    if(fd.exec() == QDialog::Rejected)
        return;
    QStringList fnl = fd.selectedFiles();
    QString fn = fnl[0];
    // note: apparently on windows, qt does you the favor of changing
    // directory separator to '/', which confuses the rest of the dll...
    // so we'll just convert them back.
    fn.replace('/', '\\');
#else
    //QString fn = QFileDialog::getOpenFileName(0, "File to send", QDir::homePath());
    QString fn = QFileDialog::getSaveFileName(this, "Save File", dt, QString(), 0);
#endif

    if(fn.length() == 0)
        return;
    if(!dwyco_copy_out_file_zap2(mid.c_str(), fn.toAscii().constData()))
    {
        QMessageBox::critical(this, "Save failed", QString("Save to file %1 failed.").arg(fn));
    }

}


void player::on_close_button_clicked()
{
    on_stop_button_clicked();
}

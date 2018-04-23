
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QtGui>
#include "ui_autoupdate.h"
#include "autoupdate.h"
#include "dlli.h"
#include "dwstr.h"
#include "pfx.h"

autoupdateform *The_autoupdateform;
static int Inhibit_unforced;
extern int DieDieDie;
void cdcxpanic(const char *);

autoupdateform::autoupdateform(QDialog *parent, Qt::WindowFlags f)
    : QDialog(parent, f | Qt::WindowStaysOnTopHint)
{
    if(The_autoupdateform)
        cdcxpanic("bad autoupdate");
    The_autoupdateform = this;
    ui.setupUi(this);
    ui.label->hide();
    ui.progressBar->hide();
    ui.shutdown_and_update_button->hide();
    ui.download_button->hide();
    ui.cancelButton->show();
    ui.cancelButton->setText("&Cancel");
    ui.status_label->hide();

    forced = 0;
    inhibit_unforced = 0;
    setAttribute(Qt::WA_QuitOnClose, 0);
}

autoupdateform::~autoupdateform()
{
    The_autoupdateform = 0;
}

static
void
DWYCOCALLCONV
dwyco_au_status_callback(int id, const char *msg, int percent_done, void *)
{
    The_autoupdateform->ui.progressBar->setValue(percent_done);
    The_autoupdateform->ui.label->setText(msg);
}

static
void
DWYCOCALLCONV
dwyco_au_download_callback(int status)
{
    const char *msg = "";
    The_autoupdateform->show();
    The_autoupdateform->ui.download_button->hide();
    switch(status)
    {
    case DWYCO_AUTOUPDATE_SIGNATURE_FAILED:
        msg = "<font color=red>Signature INVALID. Try again later. </font>";
        break;
    case DWYCO_AUTOUPDATE_DOWNLOAD_FAILED:
        msg = "<font color=red>Download failed. Try again later.</font>";
        break;
    case DWYCO_AUTOUPDATE_RUN_FAILED:
        msg = "<font color=red>Update execution failed. Try again later.</font>";
        break;
    case DWYCO_AUTOUPDATE_IN_PROGRESS:
        if(The_autoupdateform->ui.autoinstall->checkState() == Qt::Checked)
        {

            The_autoupdateform->accept();
            QCoreApplication::quit();
        }
        else
        {
            The_autoupdateform->ui.shutdown_and_update_button->show();
            The_autoupdateform->ui.cancelButton->setText("&Cancel");
        }
        break;
    }
    The_autoupdateform->ui.label->setText(msg);

}

void
autoupdateform::on_download_button_clicked()
{
    dwyco_start_autoupdate_download(dwyco_au_status_callback, 0, dwyco_au_download_callback);
    ui.cancelButton->setText("&Abort");
    ui.label->show();
    ui.progressBar->show();
    ui.progressBar->setValue(0);
    ui.download_button->hide();
}

void
autoupdateform::on_shutdown_and_update_button_clicked()
{
    QCoreApplication::quit();
}

void
autoupdateform::on_cancelButton_clicked()
{
    dwyco_abort_autoupdate_download();
    reject();
}

void
DWYCOCALLCONV
dwyco_check_for_update_done(int status, const char *desc)
{
    if(Inhibit_unforced && !The_autoupdateform->forced)
        return;
    The_autoupdateform->ui.cancelButton->setText("&Cancel");
    The_autoupdateform->ui.download_button->hide();
    switch(status)
    {
    case DWYCO_AUTOUPDATE_CHECK_FAILED:
        if(!The_autoupdateform->forced)
            return;
        The_autoupdateform->ui.textBrowser->setPlainText("Autoupdate check failed. Try again later.");
        break;
    case DWYCO_AUTOUPDATE_CHECK_NOT_NEEDED:
        if(!The_autoupdateform->forced)
            return;
        The_autoupdateform->ui.textBrowser->setPlainText("No update needed. You have the latest software.");
        break;
    case DWYCO_AUTOUPDATE_CHECK_AVAILABLE:
    case DWYCO_AUTOUPDATE_CHECK_AVAILABLE_COMPULSORY1:
    case DWYCO_AUTOUPDATE_CHECK_AVAILABLE_COMPULSORY2:

        The_autoupdateform->ui.textBrowser->setHtml(desc);
        The_autoupdateform->show();
        // note: on the mac and linux, we just give them a link
        // and make them do the update "by hand" for now.
        Inhibit_unforced = 1;
        The_autoupdateform->forced = 0;
#if defined(__WIN32__)
        The_autoupdateform->ui.download_button->show();
        The_autoupdateform->ui.download_button->setEnabled(1);
        if(status == DWYCO_AUTOUPDATE_CHECK_AVAILABLE_COMPULSORY1 ||
                status == DWYCO_AUTOUPDATE_CHECK_AVAILABLE_COMPULSORY2)
        {
            The_autoupdateform->ui.cancelButton->setEnabled(0);
            The_autoupdateform->ui.download_button->click();
        }
        else
        {
            The_autoupdateform->ui.cancelButton->setEnabled(1);
        }

#endif
        break;
    case DWYCO_AUTOUPDATE_CHECK_USER1:
    {
        // this just checks and stages an update, but doesn't run it
        dwyco_start_autoupdate_download_bg();
        QFile::remove(add_pfx(Sys_pfx, "run-update"));
        if(The_autoupdateform->forced)
        {
            The_autoupdateform->ui.textBrowser->setHtml(desc);
            The_autoupdateform->ui.cancelButton->setEnabled(1);
            The_autoupdateform->ui.autoinstall->hide();
            The_autoupdateform->ui.status_label->show();
            The_autoupdateform->ui.status_label->setText("UPDATE BEING PREPARED");
        }

        Inhibit_unforced = 1;
        The_autoupdateform->forced = 0;
        The_autoupdateform->ui.download_button->hide();
        The_autoupdateform->ui.download_button->setEnabled(0);
        The_autoupdateform->ui.cancelButton->setEnabled(1);
        The_autoupdateform->ui.cancelButton->setText("&Close");
    }

    break;
    case DWYCO_AUTOUPDATE_CHECK_USER2:
        // this creates the flag that will cause the update to be launched
        // the next time the launcher is run, in addition to downloading the
        // update. this allows us to "gate" an update so everyone gets it
        // at roughly the same time.
        // this just checks and stages an update, but doesn't run it
        if(dwyco_start_autoupdate_download_bg() == 2)
        {
            // staged and ready to go
            The_autoupdateform->ui.status_label->show();
            The_autoupdateform->ui.status_label->setText("Update is ready. QUIT and RESTART CDC-X.");
            The_autoupdateform->ui.cancelButton->hide();
            The_autoupdateform->ui.autoinstall->hide();
            The_autoupdateform->ui.shutdown_and_update_button->show();
        }
        QFile qf(add_pfx(Sys_pfx, "run-update"));
        if(qf.open(QFile::WriteOnly))
        {
            qf.putChar('r');
            qf.close();
        }
        //FILE *f = fopen("run-update", "w");
        //fclose(f);
        if(The_autoupdateform->forced)
        {
            The_autoupdateform->ui.textBrowser->setHtml(desc);
            The_autoupdateform->ui.cancelButton->setEnabled(1);
            The_autoupdateform->ui.autoinstall->hide();
            The_autoupdateform->ui.status_label->show();
            The_autoupdateform->ui.status_label->setText("UPDATE BEING PREPARED");
        }

        Inhibit_unforced = 1;
        The_autoupdateform->forced = 0;
        The_autoupdateform->ui.download_button->hide();
        The_autoupdateform->ui.download_button->setEnabled(0);
        The_autoupdateform->ui.cancelButton->setEnabled(1);
        The_autoupdateform->ui.cancelButton->setText("&Close");
        break;
    }
}


void autoupdateform::on_textBrowser_anchorClicked(const QUrl &arg1)
{
    QDesktopServices::openUrl(arg1);
    DieDieDie = 1;
}


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
#include "ui_pw.h"
#include "pw.h"
#include "dwstr.h"
#include "ssmap.h"
#include "dlli.h"
#include "tfhex.h"
#include "mainwin.h"

DwOString computer_gen_pw();
void save_low_sec(const DwOString& a);
void save_high_sec(const DwOString& a);
DwOString get_low_sec();
int check_pw_against_local_hash(const DwOString& pw);
extern DwOString My_uid;

pwform *ThePwForm;

pwform::pwform(QDialog *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    ui.fetch_pw_button->hide();

    reset();
}


static void
change_done(const char *cmd, void *user_arg, int succ, const char *failed_reason)
{
    // password is a local-only thing now. the server isn't involved
#if 0
    if(succ == 0)
    {
        QMessageBox::critical(0, "Error", failed_reason);
        ThePwForm->reset();
        return;
    }
#endif

    QByteArray qba = ThePwForm->npw.toAscii();
    DwOString a(qba.constData(), 0, qba.length());

    if(ThePwForm->ui.save_pw_checkbox->checkState() == Qt::Checked)
    {
        save_low_sec(a);
    }
    else
    {
        save_high_sec(a);
    }

    QMessageBox::information(0, "Password changed", "Password changed successfully");
    ThePwForm->reset();
}

void
pwform::reset()
{
    ui.new_pw_edit->clear();
    ui.old_pw_edit->clear();
    ui.confirm_edit->clear();
    DwOString a;
    if(!setting_get("mode", a))
    {
        a = "0";
    }

    ui.save_pw_checkbox->setCheckState(a.eq("0") ? Qt::Checked : Qt::Unchecked);
    if(a.eq("0"))
    {
        ui.new_pw_edit->setDisabled(1);
        ui.confirm_edit->setDisabled(1);
        ui.old_pw_edit->setDisabled(1);
    }
    else
    {
        ui.new_pw_edit->setDisabled(0);
        ui.confirm_edit->setDisabled(0);
        ui.old_pw_edit->setDisabled(0);
    }

    npw = "";

    QString handle;
    QString desc;
    QString loc;
    QString email;
    // note: you only ever compose for your own uid
    DwOString uid = My_uid;
    if(dwyco_info_to_display(uid, handle, desc, loc, email))
    {
        QString a("Current email: ");
        a += email;
        ui.email_label->setText(a);
    }
    else
        ui.email_label->setText("Email unavailable");
}

void
pwform::accept()
{
    QString opw = ui.old_pw_edit->text();
    DwOString m;
    int mode = 0;
    if(setting_get("mode", m))
    {
        mode = m.eq("1");
    }

    // if you are in low security mode, we let you change the
    // password or change the security mode at will.
    if(mode == 1 && !check_pw_against_local_hash(DwOString(opw.toAscii().constData(), 0,
            opw.toAscii().length())))
    {
        QMessageBox::critical(this, "Error", "Password incorrect");
        return;
    }

    // if the "start cdc without a password is checked" go into
    // low security mode

    npw = ui.new_pw_edit->text();
    if(ui.save_pw_checkbox->checkState() == Qt::Unchecked && npw.length() == 0)
    {
        QMessageBox::critical(this, "Error", "Your new password shouldn't be empty.");
        return;
    }
    QString cpw = ui.confirm_edit->text();
    if(npw != cpw && ui.save_pw_checkbox->checkState() != Qt::Checked)
    {
        QMessageBox::critical(this, "Error", "New passwords aren't the same.");
        return;
    }

    if(ui.save_pw_checkbox->checkState() == Qt::Checked)
    {
        // create a simple pw just so the server isn't
        // sitting without one
        npw = computer_gen_pw().c_str();
    }
    if(mode == 0)
    {
        opw = get_low_sec().c_str();
    }
    // just do it locally while we are upgrading
    change_done(0, 0, 1, 0);
    QDialog::accept();
}

void
pwform::reject()
{
    reset();
    QDialog::reject();
}

void
pwform::on_fetch_pw_button_clicked()
{
    Mainwinform->on_actionFetch_Lost_Password_triggered(1);
}

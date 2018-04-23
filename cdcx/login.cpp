
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
#include "ui_login.h"
#include "login.h"
#include "dwstr.h"
#include "ssmap.h"
#include "dlli.h"
#include "mainwin.h"

extern DwOString My_uid;

DwOString computer_gen_pw();
int check_pw_against_local_hash(const DwOString& pw);

loginform::loginform(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    ui.setupUi(this);
    ui.verify_label->hide();
    ui.verify_edit->hide();
    ui.save_pw_checkbox->hide();
    ui.fetch_pw_button->hide();
    ui.label_4->hide();
    ui.email_label->hide();
    ui.label_6->hide();

    is_new_account = 0;

    setWindowTitle("Password");
    // for now, don't allow them to get to the fetch password stuff
    tries = 10;
}

void
loginform::new_account()
{
    is_new_account = 1;
    ui.verify_label->show();
    ui.verify_edit->show();
    ui.save_pw_checkbox->show();
    ui.label_4->show();
    ui.email_label->show();
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
    ui.label_6->show();
    ui.buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    setWindowTitle("Create new account");
}

int
loginform::save_pw()
{
    return ui.save_pw_checkbox->checkState() == Qt::Checked;
}

DwOString
loginform::pw()
{
    QString q = ui.pw_edit->text();
    DwOString a(q.toAscii().constData(), 0, q.toAscii().length());
    return a;
}

void
loginform::accept()
{
    if(is_new_account)
    {
        if(ui.verify_edit->text() != ui.pw_edit->text())
        {
            QMessageBox::critical(this, "Passwords must match",
                                  "The two passwords you entered do not match.");
            return;
        }

        if(save_pw())
        {
            gen_pw = computer_gen_pw();
            ui.pw_edit->setText(gen_pw.c_str());
        }
        QDialog::accept();
        return;
    }
    if(save_pw())
    {
        QDialog::accept();
        return;
    }
    if(check_pw_against_local_hash(
                DwOString(ui.pw_edit->text().toAscii().constData(), 0,
                          ui.pw_edit->text().toAscii().length())))
    {
        QDialog::accept();
    }
    else
    {
        dwyco_set_local_auth(0);
        QMessageBox::critical(this, "Password Incorrect",
                              "Password Incorrect.");
        --tries;
        if(tries == 0)
        {
            reject();
        }
        return;
    }
}

void
loginform::reject()
{
    extern int DieDieDie;
    DieDieDie = 1;
    QDialog::reject();
}

void
loginform::on_fetch_pw_button_clicked()
{
    Mainwinform->on_actionFetch_Lost_Password_triggered(1);
}

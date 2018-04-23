
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef QWIZ_H
#define QWIZ_H
#include <QWizardPage>
#include <QDialog>
#include <QMessageBox>
#include "ui_userwid.h"
#include "ui_netspeedwid.h"
#include "ui_toswarn.h"
#include "qval.h"
QByteArray fname();

// this is a necessitas/android hack
class activateit : public QWizardPage
{
public:
    void showEvent(QShowEvent *e) {
        QWizardPage::activateWindow();
        QWizardPage::showEvent(e);
    }
};

class toswarn_page : public activateit
{
    Q_OBJECT
public:
    toswarn_page() {
        ui.setupUi(this);
        setTitle("Terms of Service");

    }
public slots:
    void on_pushButton_clicked() {
        exit(0);
    }
private:
    Ui::toswarn ui;
};

class userdata_page : public activateit
{
    Q_OBJECT
public:
    userdata_page() {
        ui.setupUi(this);
        ui.desc->setVisible(0);
        ui.label_3->setVisible(0);
        setTitle("Profile information");
        //setSubTitle("Please enter the information to display in your profile");
        registerField("name", ui.name);
        registerField("location", ui.location);
        registerField("email", ui.email);

        ui.name->setValidator(size_validator());
        ui.email->setValidator(email_validator());
        ui.name->setText(fname());
    }
    bool validatePage() {
        if(ui.email->text().length() > 0 && !ui.email->hasAcceptableInput())
        {
            QMessageBox::critical(this, "Bad Email", "Please enter a valid email address.");
            return 0;
        }
        if(!ui.name->hasAcceptableInput())
        {
            QMessageBox::critical(this, "Handle", "Please enter a handle with at least 3 characters");
            return 0;
        }
        return 1;
    }

    Ui::userwid ui;
};

class netspeed_page : public activateit
{
    Q_OBJECT
public:
    netspeed_page() {
        ui.setupUi(this);
        setTitle("Internet Speed");
        setSubTitle("Click the button below to start the speed test, or enter your network speed in kilobits per second.");
        registerField("upload*", ui.upload_speed);
        registerField("download*", ui.download_speed);
        ui.upload_speed->setValidator(new QIntValidator(1, 1000000, 0));
        ui.download_speed->setValidator(new QIntValidator(1, 1000000, 0));
    }
    bool validatePage() {
        if(!ui.upload_speed->hasAcceptableInput() || !ui.download_speed->hasAcceptableInput())
        {
            QMessageBox::critical(this, "Speed", "Please run the speed test, or enter valid numbers.");
            return 0;
        }
        return 1;
    }

public slots:
    void on_run_bw_test_button_clicked();

public:
    Ui::netspeedwid ui;
};


void dowiz();

#endif

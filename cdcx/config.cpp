
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QtGui>
#include <QMessageBox>
#include <QFileDialog>
#include <QRegularExpression>
#include <QDialog>
#include "dwycolistscoped.h"
#include "ui_config.h"
#include "config.h"
#include "dlli.h"
#include "ssmap.h"
#include "mainwin.h"
#include "simple_public.h"
#include "tfhex.h"
extern DwOString My_uid;

configform *TheConfigForm;
extern int Display_archived_users;
extern int User_count;
extern int Inhibit_powerclean;

void cdcxpanic(char *);

configform::configform(QDialog *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    // this is handled automatically now
    // so we don't allow the user to modify it.
    ui.label_11->hide();
    ui.not_usedcdc_net__pal_port->hide();
    ui.CDC_zap__always_accept->hide();
    ui.CDC_call_acceptance__pw->hide();
    ui.CDC_call_acceptance__require_pw->hide();
    ui.CDC_call_acceptance__max_audio->hide();
    ui.CDC_call_acceptance__max_audio_recv->hide();
    ui.CDC_call_acceptance__max_video->hide();
    ui.CDC_call_acceptance__max_video_recv->hide();
    ui.label_6->hide();
    ui.label_7->hide();
    ui.label_8->hide();
    ui.label_9->hide();
    ui.label_10->hide();
    ui.untrash->hide();
    ui.empty_trash->hide();
    ui.sync_table->setHorizontalHeaderLabels({"UID", "IP", "STAT"});

    TheConfigForm = this;
    // mainwinform may not exist yet
    if(Mainwinform)
    {
        connect(this, SIGNAL(content_filter_event(int)), Mainwinform, SIGNAL(content_filter_event(int)));
        connect(this, SIGNAL(pals_only(int)), Mainwinform, SIGNAL(pals_only(int)));
    }
    setAttribute(Qt::WA_QuitOnClose, 0);
    load_untrash_button();
}

void
configform::accept()
{
    QDialog::accept();
    // transfer all values to the dll as needed

    {
        QList<QCheckBox *> cb;
        QRegularExpression r = QRegularExpression::fromWildcard(QString("CDC_*"), Qt::CaseSensitive);
        cb = findChildren<QCheckBox *>(r);
        for(int i = 0; i < cb.count(); ++i)
        {
            QCheckBox *c = cb.value(i);
            QString onm = c->objectName();
            onm.remove("CDC_");
            onm.replace("__", "/");
            if(!dwyco_set_setting(onm.toAscii().constData(), c->checkState() == Qt::Checked ? "1" : "0"))
                cdcxpanic("can't set setting");
        }
    }
    {
        QList<QLineEdit *> cb;
        QRegularExpression r = QRegularExpression::fromWildcard(QString("CDC_*"), Qt::CaseSensitive);
        cb = findChildren<QLineEdit *>(r);
        for(int i = 0; i < cb.count(); ++i)
        {
            QLineEdit *c = cb.value(i);
            QString onm = c->objectName();
            onm.remove("CDC_");
            onm.replace("__", "/");
            if(!dwyco_set_setting(onm.toAscii().constData(), c->text().toAscii().constData()))
                cdcxpanic("can't set setting");
        }
    }
    // this doesn't have any meaning in cdc-x, since we don't give
    // them the option to leave msgs on the server or anything.
    // so we always make them accept.
    dwyco_set_setting("zap/always_accept", "1");
    // TCP only
    dwyco_set_setting("net/call_setup_media_select", "1");

    // update the pals-only settings
    if(ui.pals_only->isChecked())
    {
        dwyco_set_pals_only(1);
    }
    else
    {
        dwyco_set_pals_only(0);
    }
    emit pals_only(ui.pals_only->isChecked());

    int agc;
    int denoise;
    double audio_delay;
    agc = ui.agc->isChecked();
    denoise = ui.denoise->isChecked();
    audio_delay = ui.audio_delay->value();
    dwyco_set_codec_data(agc, denoise, audio_delay);

    // b/w settings
    //int up = ui.upload_speed->text().toInt();
    //int down = ui.download_speed->text().toInt();
    //dwyco_set_rate_tweaks(20, 65535, up, down);
    dwyco_set_setting("rate/max_fps", "20");
    dwyco_set_setting("rate/kbits_per_sec_out", ui.upload_speed->text().toLatin1());
    dwyco_set_setting("rate/kbits_per_sec_in", ui.download_speed->text().toLatin1());

    setting_put("chat_dont_display_video", ui.chat_dont_display_video->isChecked());
    if(simple_public::Simple_publics.count() > 0)
    {
        simple_public *s = (simple_public *)(void*)simple_public::Simple_publics[0];
        s->set_hide_video(ui.chat_dont_display_video->isChecked());
    }
    setting_put("call_acceptance/max_audio", ui.CDC_call_acceptance__max_audio->text());
    setting_put("call_acceptance/max_audio_recv", ui.CDC_call_acceptance__max_audio_recv->text());
    setting_put("mute_alerts", ui.mute_alerts->isChecked());
    //setting_put("chat_dont_show_pics", ui.chat_dont_show_pics->isChecked());
    setting_put("chat_pic_size", ui.picsize_combobox->currentIndex());
    setting_put("profiles_no_auto_load", ui.profiles_no_auto_load->isChecked());
    int old = 0;
    setting_get("chat_show_unreviewed", old);
    setting_put("chat_show_unreviewed", ui.chat_show_all->isChecked());
    if(!!old != ui.chat_show_all->isChecked())
    {
        emit content_filter_event(!old);
    }

    setting_put("disable_backups", ui.disable_backups->isChecked());
    setting_put("disable_bg", ui.inhibit_bg_checkbox->isChecked());

    settings_save();
}

void
configform::reject()
{
    QDialog::reject();
    load();
}

void
configform::load()
{
    {
        QList<QCheckBox *> cb;
        QRegularExpression r = QRegularExpression::fromWildcard(QString("CDC_*"), Qt::CaseSensitive);
        cb = findChildren<QCheckBox *>(r);
        for(int i = 0; i < cb.count(); ++i)
        {
            QCheckBox *c = cb.value(i);
            QString onm = c->objectName();
            onm.remove("CDC_");
            onm.replace("__", "/");
            const char *value;
            int type;
            int len;
            if(!dwyco_get_setting(onm.toAscii().constData(), &value, &len, &type))
                cdcxpanic("can't set setting");
            if(type != DWYCO_TYPE_INT)
                cdcxpanic("something is really hosed");
            c->setCheckState(atol(value) ? Qt::Checked : Qt::Unchecked);
        }
    }
    {
        QList<QLineEdit *> cb;
        QRegularExpression r = QRegularExpression::fromWildcard(QString("CDC_*"), Qt::CaseSensitive);
        cb = findChildren<QLineEdit *>(r);
        for(int i = 0; i < cb.count(); ++i)
        {
            QLineEdit *c = cb.value(i);
            QString onm = c->objectName();
            onm.remove("CDC_");
            onm.replace("__", "/");
            const char *value;
            int type;
            int len;
            if(!dwyco_get_setting(onm.toAscii().constData(), &value, &len, &type))
                cdcxpanic("can't set setting");
            else if(type != DWYCO_TYPE_STRING && type != DWYCO_TYPE_INT)
                cdcxpanic("something is hosed");
            c->setText(QByteArray(value, len));
        }
    }
    // load misc stuff
    if(dwyco_get_pals_only())
    {
        ui.pals_only->setChecked(1);
        //ui.CDC_zap__send_auto_reply->setEnabled(1);
    }
    else
    {
        ui.pals_only->setChecked(0);
        //ui.CDC_zap__send_auto_reply->setEnabled(0);
    }


    if(ui.CDC_net__advertise_nat_ports->isChecked())
    {
        ui.CDC_net__nat_primary_port->setEnabled(1);
        ui.CDC_net__nat_secondary_port->setEnabled(1);
    }
    else
    {
        ui.CDC_net__nat_primary_port->setEnabled(0);
        ui.CDC_net__nat_secondary_port->setEnabled(0);
    }
    int agc;
    int denoise;
    double audio_delay;
    dwyco_get_codec_data(&agc, &denoise, &audio_delay);
    ui.agc->setChecked(agc);
    ui.denoise->setChecked(denoise);
    ui.audio_delay->setValue(audio_delay);


    {
    const char *val;
    int len;
    int tp;
    dwyco_get_setting("rate/kbits_per_sec_out", &val, &len, &tp);
    ui.upload_speed->setText(val);
    dwyco_get_setting("rate/kbits_per_sec_in", &val, &len, &tp);
    ui.download_speed->setText(val);
    }


    int val;

    if(!setting_get("chat_dont_display_video", val))
    {
        setting_put("chat_dont_display_video", 0);
        val = 0;
    }
    ui.chat_dont_display_video->setChecked(val);

    if(!setting_get("mute_alerts", val))
    {
        setting_put("mute_alerts", 0);
        val = 0;
    }
    ui.mute_alerts->setChecked(val);

    if(!setting_get("chat_dont_show_pics", val))
    {
        setting_put("chat_dont_show_pics", 0);
        val = 0;
    }
    //ui.chat_dont_show_pics->setChecked(val);
    int nval;
    if(!setting_get("chat_pic_size", nval))
    {
        // import previous setting
        nval = 2;
        if(val == 1)
            nval = 0;
        else if(val == 1)
            nval = 2; // default to large images on upgrade
        setting_put("chat_pic_size", nval);
    }
    ui.picsize_combobox->setCurrentIndex(nval);

    if(!setting_get("profiles_no_auto_load", val))
    {
        setting_put("profiles_no_auto_load", 0);
        val = 0;
    }
    ui.profiles_no_auto_load->setChecked(val);

    if(!setting_get("chat_show_unreviewed", val))
    {
        setting_put("chat_show_unreviewed", 0);
        val = 0;
    }
    ui.chat_show_all->setChecked(val);
    QString a(to_hex(My_uid).c_str());
    a.truncate(10);
    ui.uid->setText(a);

    if(!setting_get("disable_backups", val))
    {
        setting_put("disable_backups", 0);
        val = 0;
    }
    ui.disable_backups->setChecked(val);

    if(!setting_get("disable_bg", val))
    {
        setting_put("disable_bg", 0);
        val = 0;
    }
    ui.inhibit_bg_checkbox->setChecked(val);

    if(ui.CDC_group__alt_name->text().length() == 0)
    {
        DWYCO_LIST gs;
        if(dwyco_get_group_status(&gs))
        {

            simple_scoped qgs(gs);
            if(qgs.get_long(DWYCO_GS_VALID) != 1)
            {
                if(qgs.get_long(DWYCO_GS_IN_PROGRESS) == 1)
                    ui.sync_enable->setChecked(true);

            }
            else
                ui.sync_enable->setChecked(false);
        }

        ui.CDC_group__alt_name->setReadOnly(false);
    }
    else
    {
        ui.sync_enable->setChecked(true);
        ui.CDC_group__alt_name->setReadOnly(true);
    }

}


void
configform::on_CDC_zap__no_forward_default_stateChanged(int newstate)
{
}

void
configform::on_pal_auth_stateChanged(int newstate)
{
    //dwyco_set_pal_auth_state(newstate == Qt::Checked, 0, 0);
}

void
configform::on_run_bw_test_button_clicked()
{
    int bw_in;
    int bw_out;

    dwyco_estimate_bandwidth2(&bw_out, &bw_in);
    QString in;
    in.setNum(bw_in / 1000);
    QString out;
    out.setNum(bw_out / 1000);
    ui.upload_speed->setText(out);
    ui.download_speed->setText(in);
    QMessageBox::information(0, "Bandwidth report", QString(
                                 "Download %1 kbps, Upload %2 kbps").arg(in, out));
}


void configform::on_CDC_net__primary_port_cursorPositionChanged(int , int )
{

}

void configform::on_create_auto_reply_clicked()
{
#if 0
    composer *c = new composer_autoreply(Mainwinform);
    //c->set_uid(uid);
    c->show();
    c->raise();
#endif
}

void configform::on_revert_auto_reply_clicked()
{
#if 0
    dwyco_set_auto_reply_msg(0, 0, 0);
#endif
}

void configform::on_CDC_call_acceptance__max_audio_textChanged(QString val)
{
    //settings_put("call_acceptance/max_audio", val.toAscii().constData());
}


void configform::on_empty_trash_clicked()
{
    dwyco_empty_trash();
    load_untrash_button();
}

void configform::on_untrash_clicked()
{
    dwyco_untrash_users();
    load_untrash_button();
    dwyco_load_users2(!Display_archived_users, &User_count);
    Mainwinform->load_users();
    cdcx_set_refresh_users(1);
    // don't automatically cleanup on exit, this gives user
    // a chance to look things over on next restart.
    Inhibit_powerclean = 1;
}

void
configform::load_untrash_button()
{
    int n = dwyco_count_trashed_users();
    ui.untrash->setText(QString("Untrash %1 users").arg(n));
}

void configform::on_reset_backup_button_clicked()
{
    dwyco_remove_backup();
    QMessageBox msgBox(QMessageBox::Information, "Backup redo",
                       "Your backup has been reset. "
                       "A new backup will be created the next time you exit CDC-X (unless you have "
                       "disabled backups.) WARNING: this does NOT delete the backups in your Documents folder. You must do that manually.",
                       QMessageBox::Ok, this);
    msgBox.setWindowFlag(Qt::WindowStaysOnTopHint, true);
    msgBox.exec();
}

static
QString
get_a_backup_filename(QWidget *parent, QString filter)
{
    static QString last_directory;
    if(last_directory.length() == 0)
    {
#ifdef DWYCO_QT5
        QStringList sl = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        last_directory = sl[0];
#else
        last_directory = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#endif
    }
#if (!defined(LINUX) && !defined(MAC_CLIENT))
// windows wants to change the cwd on us, so tell it to
// use the qt dialog.
    // apparently despite our request otherwise, qt decides to use the native dialog
    //QString fn = QFileDialog::getOpenFileName(0, "File to send", QString(), QString(), 0, QFileDialog::DontUseNativeDialog);

    QFileDialog fd;

    fd.setOption(QFileDialog::DontUseNativeDialog);
    fd.setDirectory(last_directory);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setNameFilter(filter);

    if(fd.exec() == QDialog::Rejected)
        return "";
    QStringList fnl = fd.selectedFiles();
    QString fn = fnl[0];
    if(fn.length() > 0)
    {
        last_directory = fn;
        last_directory.truncate(last_directory.lastIndexOf("/"));
    }

    // note: apparently on windows, qt does you the favor of changing
    // directory separator to '/', which confuses the rest of the dll...
    // so we'll just convert them back.

    fn.replace('/', '\\');
#else


    QString fn = QFileDialog::getOpenFileName(parent, "Select file", last_directory, filter);
    if(fn.length() > 0)
    {
        last_directory = fn;
        last_directory.truncate(last_directory.lastIndexOf("/"));

    }
#endif

    return fn;
}

void configform::on_restore_button_clicked()
{
    if(ui.sync_enable->isChecked())
    {
        QMessageBox mb(QMessageBox::Information, "Restore backup",
                       QString("You cannot restore a backup while linked to a device group. "
                               "First, unlink this device from the group, then restart and "
                               "perform the restore operation. Then restart and re-link to the "
                               "group."),
                       QMessageBox::Ok, this);
        mb.exec();
        return;
    }
    QMessageBox mb(QMessageBox::Warning, "Restore backup",
                   QString("Are you sure you want to restore from backup file? Restoring "
                           "messages simply adds the messages in the backup to your current "
                           "CDC-X account. This is fairly safe but may take some time. IF "
                           "YOU CLICK \"Restore Account Info\", both messages and your old account "
                           "are restored. CDC-X must exit immediately after this, and it may take "
                           "some time to complete the restore. Please be patient. "
                           "(NO UNDO)"),
                   QMessageBox::No, this);
    QPushButton *acct = mb.addButton("Restore Account Info", QMessageBox::ApplyRole);
    QPushButton *msgs = mb.addButton("Restore messages only", QMessageBox::YesRole);
    mb.setDefaultButton(QMessageBox::No);
    int s = mb.exec();
    if(s == QMessageBox::No)
    {
        return;
    }
    int msgs_only = 0;
    if(mb.clickedButton() == msgs)
        msgs_only = 1;

    QString bufn = get_a_backup_filename(&mb, "*.sql");
    if(bufn.length() == 0)
        return;

    if(QMessageBox::information(this, "Restore",
                                "CDC-X will exit when the restore is complete. Click OK to continue.",
                                QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Cancel)
        return;

    if(dwyco_restore_from_backup(bufn.toLatin1().constData(), msgs_only))
    {
        DieDieDie = 1;
        return;
    }


    QMessageBox::information(this, "Restore",
                             "Restore encountered an error. Don't worry, most of it may have worked fine. Click OK to exit. Then restart CDC-X.",
                             QMessageBox::Ok);
    DieDieDie = 1;
}

void configform::on_pals_only_clicked(bool checked)
{
    if(checked)
        dwyco_field_debug("cfg-pals-only", 1);
}

void configform::on_sync_enable_clicked(bool checked)
{
    if(checked)
    {
        QString gtrim = ui.CDC_group__alt_name->text().trimmed();
        QString key = ui.CDC_group__join_key->text();
        if(gtrim.toLatin1().length() < 4 ||
            key.toLatin1().length() < 4)
        {
            QMessageBox warn(QMessageBox::Warning, "Device linking failed",
                             "The device name and password must be at least 4 characters.",
                             QMessageBox::Ok);
            warn.exec();
            ui.sync_enable->setChecked(false);
            ui.CDC_group__join_key->setText("");
            ui.CDC_group__alt_name->setReadOnly(false);
            return;
        }
        dwyco_set_setting("group/join_key", key.toLatin1().constData());

        if(!dwyco_start_gj2(gtrim.toLatin1().constData(), key.toLatin1().constData()))
        {
            QMessageBox warn(QMessageBox::Warning, "Device linking failed",
                                     "Can't perform linking now, try again later.",
                                     QMessageBox::Ok);
            warn.exec();
        }
        else
        {
            ui.CDC_group__alt_name->setReadOnly(true);
            ui.sync_enable->setText(ui.sync_enable->text() + "(Working...)");
        }
    }
    else
    {
        //dwyco_set_setting("group/join_key", "");
        if(!dwyco_start_gj2("", ""))
        {
            QMessageBox::information(this, "Device UNLINKING failed",
                                     "Can't perform UNLINKING now, try again later.",
                                     QMessageBox::Ok);
        }
        ui.CDC_group__join_key->setText("");
        ui.CDC_group__alt_name->setReadOnly(false);
    }

   }

void configform::on_show_password_clicked(bool checked)
{
    if(checked)
    {
        ui.CDC_group__join_key->setEchoMode(QLineEdit::Normal);
    }
    else
    {
        ui.CDC_group__join_key->setEchoMode(QLineEdit::Password);
    }
}

void
configform::showEvent(QShowEvent *ev)
{
    QDialog::showEvent(ev);

    DWYCO_LIST gs;
    if(!dwyco_get_group_status(&gs))
        return;
    simple_scoped qgs(gs);
    if(qgs.get_long(DWYCO_GS_VALID) != 1)
    {
        if(qgs.get_long(DWYCO_GS_IN_PROGRESS) == 0)
            return;
        ui.sync_enable->setText("Enable device linking (Working...)");
        return;
    }
    long percent = qgs.get_long(DWYCO_GS_PERCENT_SYNCED);

    DWYCO_LIST status;
    if(!dwyco_get_sync_model(&status))
        return;
    simple_scoped qstatus(status);
    int n = qstatus.rows();
    int c = 0;
    for(int i = 0; i < n; ++i)
    {
        QByteArray b = qstatus.get<QByteArray>(i, DWYCO_SM_STATUS);
        if(b.at(1) == 'a')
            ++c;
    }

    ui.sync_enable->setText("Enable device linking (" + QString::number(percent) + "% synced, " + QString::number(c) + "/" + QString::number(n) + " active)");

}



void configform::on_sync_refresh_button_clicked()
{
    auto st = ui.sync_table;

    st->clearContents();

    DWYCO_SYNC_MODEL sm;
    if(!dwyco_get_sync_model(&sm))
        return;

    simple_scoped qsm(sm);

    // Get the total number of items from the API.
    int numRows = qsm.rows();
    st->setRowCount(numRows);

    // Create a QStandardItem for each column in each row.
    for (int i = 0; i < numRows; ++i)
    {
        // Fetch data for each column from the API
        auto w = new QTableWidgetItem(qsm.get<QByteArray>(i, DWYCO_SM_UID).toHex());
        st->setItem(i, 0, w);
        w = new QTableWidgetItem(qsm.get<QByteArray>(i, DWYCO_SM_IP));
        st->setItem(i, 1, w);
        auto stat = qsm.get<QByteArray>(i, DWYCO_SM_STATUS);

        w = new QTableWidgetItem();
        if(stat.at(0) == 'o')
            w->setText("->");
        else
            w->setText("<-");

        if(stat.at(1) == 'a') {
            w->setBackground(QBrush(Qt::green));
            bool p = qsm.is_nil(i, DWYCO_SM_PROXY);
            if(!p)
                w->setBackground(QBrush(Qt::cyan));

        }
        else if(stat.at(1) == 'i') {
             w->setBackground(QBrush(Qt::yellow));
        }
        else if(stat.at(1) == 'd') {
            w->setBackground(QBrush(Qt::white));
        }

        st->setItem(i, 2, w);
    }

}


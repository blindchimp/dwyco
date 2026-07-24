
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
#include <QClipboard>
#include <QGuiApplication>
#include "dwycolistscoped.h"
#include "ui_config.h"
#include "config.h"
#include "dlli.h"
#include "ssmap.h"
#include "mainwin.h"
#include "simple_public.h"
#include "simple_call.h"
#include "tfhex.h"
extern DwOString My_uid;

configform *TheConfigForm;
extern int Display_archived_users;
extern int User_count;
extern int Inhibit_powerclean;

void cdcxpanic(char *);

#ifdef DWYCO_TOXCORE
int s_tox_connected = 0;

static int tox_to_idx(const QByteArray &status)
{
    if(status == "away") return 1;
    if(status == "busy") return 2;
    return 0;
}

static QByteArray idx_to_status(int idx)
{
    if(idx == 1) return "away";
    if(idx == 2) return "busy";
    return "none";
}

static QColor tox_badge_color_for_info(const QByteArray &status, const QByteArray &user_status)
{
    if(status == "offline")
        return QColor(0x9E, 0x9E, 0x9E);
    if(user_status == "busy")
        return QColor(0xF4, 0x43, 0x36);
    if(user_status == "away")
        return QColor(0xFF, 0x98, 0x00);
    return QColor(0x4C, 0xAF, 0x50);
}

static QPixmap make_tox_badge(int size, QColor color)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawEllipse(1, 1, size - 2, size - 2);
    p.setPen(Qt::white);
    QFont f;
    f.setBold(true);
    f.setPixelSize(size * 0.6);
    p.setFont(f);
    p.drawText(QRect(0, 0, size, size), Qt::AlignCenter, "T");
    p.end();
    return pm;
}
#endif

configform::configform(QDialog *parent)
    : QDialog(parent)
    , tox_refresh_timer(0)
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

#ifdef DWYCO_TOXCORE
    if(!dwyco_tox_available())
    {
        int idx = ui.tabWidget->indexOf(ui.tab_tox);
        if(idx >= 0)
            ui.tabWidget->removeTab(idx);
    }
    else
    {
        tox_refresh_timer = new QTimer(this);
        connect(tox_refresh_timer, &QTimer::timeout, this, &configform::refresh_tox_tab);
        connect(ui.tox_friend_list, &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem *item) {
                QModelIndex idx = ui.tox_friend_list->model()->index(ui.tox_friend_list->row(item), 0);
                on_tox_friend_list_doubleClicked(idx);
            });
    }
#else
    {
        int idx = ui.tabWidget->indexOf(ui.tab_tox);
        if(idx >= 0)
            ui.tabWidget->removeTab(idx);
    }
#endif

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
    dwyco_set_setting("rate/max_fps", "20");
    dwyco_set_setting("rate/kbits_per_sec_out", ui.upload_speed->text().toLatin1());
    dwyco_set_setting("rate/kbits_per_sec_in", ui.download_speed->text().toLatin1());

    setting_put("chat_dont_display_video", ui.chat_dont_display_video->isChecked(), 0);
    if(simple_public::Simple_publics.count() > 0)
    {
        simple_public *s = (simple_public *)(void*)simple_public::Simple_publics[0];
        s->set_hide_video(ui.chat_dont_display_video->isChecked());
    }
    setting_put("call_acceptance/max_audio", ui.CDC_call_acceptance__max_audio->text(), 0);
    setting_put("call_acceptance/max_audio_recv", ui.CDC_call_acceptance__max_audio_recv->text(), 0);
    setting_put("mute_alerts", ui.mute_alerts->isChecked(), 0);
    setting_put("chat_pic_size", ui.picsize_combobox->currentIndex(), 0);
    setting_put("profiles_no_auto_load", ui.profiles_no_auto_load->isChecked(), 0);
    int old = 0;
    setting_get("chat_show_unreviewed", old);
    setting_put("chat_show_unreviewed", ui.chat_show_all->isChecked(), 0);
    if(!!old != ui.chat_show_all->isChecked())
    {
        emit content_filter_event(!old);
    }

    setting_put("disable_backups", ui.disable_backups->isChecked(), 0);
    setting_put("disable_bg", ui.inhibit_bg_checkbox->isChecked(), 0);

#ifdef DWYCO_TOXCORE
    if(ui.tabWidget->indexOf(ui.tab_tox) >= 0)
    {
        int tox_was_enabled = 0;
        setting_get("tox_enabled", tox_was_enabled);
        int tox_now = ui.tox_enable->isChecked() ? 1 : 0;
        setting_put("tox_enabled", tox_now, 0);

        if(tox_now && !tox_was_enabled)
        {
            dwyco_enable_tox("tox_save.tox");
        }
        else if(!tox_now && tox_was_enabled)
        {
            dwyco_disable_tox();
        }

        setting_put("auto_away_enabled", ui.auto_away_enable->isChecked() ? 1 : 0, 0);
        int timeout_values[] = {60, 120, 300, 600, 900, 1800};
        int tidx = ui.auto_away_timeout->currentIndex();
        if(tidx >= 0 && tidx < 6)
            setting_put("auto_away_timeout", timeout_values[tidx], 0);

        QByteArray name_bytes = ui.tox_name->text().toUtf8();
        QByteArray status_bytes = ui.tox_status_msg->text().toUtf8();
        dwyco_tox_set_name(name_bytes.constData(), name_bytes.length());
        dwyco_tox_set_status_message(status_bytes.constData(), status_bytes.length());

        QByteArray status_str = idx_to_status(ui.tox_user_status->currentIndex());
        dwyco_tox_set_user_status(status_str.constData());
    }
#endif

    settings_save2();
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
    }
    else
    {
        ui.pals_only->setChecked(0);
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

#ifdef DWYCO_TOXCORE
    if(ui.tabWidget->indexOf(ui.tab_tox) >= 0)
    {
        int tox_val;
        if(!setting_get("tox_enabled", tox_val))
            tox_val = 0;
        ui.tox_enable->setChecked(tox_val);
        set_tox_widgets_enabled(tox_val && dwyco_tox_available());

        if(!setting_get("auto_away_enabled", tox_val))
            tox_val = 0;
        ui.auto_away_enable->setChecked(tox_val);

        int aa_timeout;
        if(!setting_get("auto_away_timeout", aa_timeout))
            aa_timeout = 300;
        int timeout_values[] = {60, 120, 300, 600, 900, 1800};
        int tidx = 2;
        for(int i = 0; i < 6; ++i)
        {
            if(timeout_values[i] == aa_timeout)
            {
                tidx = i;
                break;
            }
        }
        ui.auto_away_timeout->setCurrentIndex(tidx);
        ui.auto_away_timeout->setEnabled(tox_val && ui.auto_away_enable->isChecked());

        char *name_out = 0;
        int name_len = 0;
        if(dwyco_tox_get_name(&name_out, &name_len))
            ui.tox_name->setText(QByteArray(name_out, name_len));
        else
            ui.tox_name->setText("");

        char *status_out = 0;
        int status_len = 0;
        if(dwyco_tox_get_status_message(&status_out, &status_len))
            ui.tox_status_msg->setText(QByteArray(status_out, status_len));
        else
            ui.tox_status_msg->setText("");

        char *addr_out = 0;
        int addr_len = 0;
        if(dwyco_tox_get_self_address(&addr_out, &addr_len))
        {
            QByteArray addr_hex = QByteArray(addr_out, addr_len).toHex();
            ui.tox_id_display->setText(addr_hex.left(16) + "...");
        }

        char *us_out = 0;
        int us_len = 0;
        if(dwyco_tox_get_user_status(&us_out, &us_len))
        {
            QByteArray us(us_out, us_len);
            ui.tox_user_status->setCurrentIndex(tox_to_idx(us));
        }
        else
        {
            ui.tox_user_status->setCurrentIndex(0);
        }

        update_tox_status_indicator();
        refresh_tox_friend_list();

        if(tox_val)
            tox_refresh_timer->start(5000);
        else
            tox_refresh_timer->stop();
    }
#endif

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

    int numRows = qsm.rows();
    st->setRowCount(numRows);

    for (int i = 0; i < numRows; ++i)
    {
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

// --- Tox tab implementation ---

#ifdef DWYCO_TOXCORE
void configform::set_tox_widgets_enabled(bool enabled)
{
    ui.tox_status_indicator->setEnabled(enabled);
    ui.tox_status_label->setEnabled(enabled);
    ui.tox_status_label_2->setEnabled(enabled);
    ui.tox_user_status->setEnabled(enabled);
    ui.auto_away_enable->setEnabled(enabled);
    ui.auto_away_timeout->setEnabled(enabled && ui.auto_away_enable->isChecked());
    ui.tox_name->setEnabled(enabled);
    ui.tox_status_msg->setEnabled(enabled);
    ui.tox_update_name->setEnabled(enabled);
    ui.tox_copy_id->setEnabled(enabled);
    ui.tox_add_friend_label->setEnabled(enabled);
    ui.tox_friend_id_input->setEnabled(enabled);
    ui.tox_add_friend->setEnabled(enabled);
    ui.tox_friends_label->setEnabled(enabled);
    ui.tox_friend_list->setEnabled(enabled);
    ui.tox_delete_friend->setEnabled(enabled);
}

void configform::update_tox_status_indicator()
{
    QPalette pal = ui.tox_status_indicator->palette();
    if(s_tox_connected)
    {
        pal.setColor(QPalette::Window, Qt::green);
        ui.tox_status_label->setText("Connected");
    }
    else
    {
        pal.setColor(QPalette::Window, Qt::red);
        ui.tox_status_label->setText("Not connected");
    }
    ui.tox_status_indicator->setAutoFillBackground(true);
    ui.tox_status_indicator->setPalette(pal);
}

void configform::refresh_tox_tab()
{
    update_tox_status_indicator();
    refresh_tox_friend_list();
}

void configform::refresh_tox_friend_list()
{
    ui.tox_friend_list->clear();

    DWYCO_TOX_FRIENDS_MODEL fl = 0;
    if(!dwyco_tox_get_friends_model(&fl))
        return;
    simple_scoped qfl(fl);
    int n = qfl.rows();
    for(int i = 0; i < n; ++i)
    {
        QByteArray pk = qfl.get<QByteArray>(i, DWYCO_TF_PUBKEY);
        QByteArray pkhex = pk.toHex();
        QByteArray name = qfl.get<QByteArray>(i, DWYCO_TF_NAME);
        QByteArray status = qfl.get<QByteArray>(i, DWYCO_TF_STATUS);
        QByteArray user_status = qfl.get<QByteArray>(i, DWYCO_TF_USER_STATUS);

        QColor badge_color = tox_badge_color_for_info(status, user_status);
        QPixmap badge = make_tox_badge(16, badge_color);

        QString display = QString(name) + "  " + pkhex.left(16);
        QListWidgetItem *item = new QListWidgetItem(badge, display, ui.tox_friend_list);
        item->setData(Qt::UserRole, pkhex);
    }
}

void configform::on_tox_enable_toggled(bool checked)
{
    set_tox_widgets_enabled(checked && dwyco_tox_available());
    if(checked)
    {
        if(!dwyco_tox_available())
            return;
        dwyco_enable_tox("tox_save.tox");

        char *name_out = 0;
        int name_len = 0;
        if(dwyco_tox_get_name(&name_out, &name_len))
            ui.tox_name->setText(QByteArray(name_out, name_len));

        char *status_out = 0;
        int status_len = 0;
        if(dwyco_tox_get_status_message(&status_out, &status_len))
            ui.tox_status_msg->setText(QByteArray(status_out, status_len));

        tox_refresh_timer->start(5000);
    }
    else
    {
        dwyco_disable_tox();
        tox_refresh_timer->stop();
        s_tox_connected = 0;
        update_tox_status_indicator();
        ui.tox_friend_list->clear();
    }
}

void configform::on_tox_update_name_clicked()
{
    QByteArray name_bytes = ui.tox_name->text().toUtf8();
    QByteArray status_bytes = ui.tox_status_msg->text().toUtf8();
    dwyco_tox_set_name(name_bytes.constData(), name_bytes.length());
    dwyco_tox_set_status_message(status_bytes.constData(), status_bytes.length());
}

void configform::on_tox_copy_id_clicked()
{
    char *addr_out = 0;
    int addr_len = 0;
    if(dwyco_tox_get_self_address(&addr_out, &addr_len))
    {
        QByteArray addr_hex = QByteArray(addr_out, addr_len).toHex();
        QGuiApplication::clipboard()->setText(addr_hex);
    }
}

void configform::on_tox_add_friend_clicked()
{
    QString id_str = ui.tox_friend_id_input->text().trimmed();
    if(id_str.length() == 0)
        return;
    QByteArray id_bytes = id_str.toLatin1();
    QByteArray msg = "Hello from CDC-X!";
    dwyco_tox_add_friend(id_bytes.constData(), id_bytes.length(), msg.constData());
    ui.tox_friend_id_input->setText("");
    refresh_tox_friend_list();
}

void configform::on_tox_delete_friend_clicked()
{
    QListWidgetItem *item = ui.tox_friend_list->currentItem();
    if(!item)
        return;

    QMessageBox mb(QMessageBox::Warning, "Delete Friend",
                   "Delete this friend and remove them from your contact list?",
                   QMessageBox::Ok | QMessageBox::Cancel, this);
    if(mb.exec() != QMessageBox::Ok)
        return;

    QByteArray pkhex = item->data(Qt::UserRole).toByteArray();
    QByteArray pk = QByteArray::fromHex(pkhex);
    dwyco_tox_delete_friend(pk.constData(), pk.length());
    refresh_tox_friend_list();
}

void configform::on_tox_user_status_changed(int index)
{
    QByteArray status_str = idx_to_status(index);
    dwyco_tox_set_user_status(status_str.constData());
}

void configform::on_tox_friend_list_doubleClicked(const QModelIndex &index)
{
    if(!index.isValid())
        return;
    QListWidgetItem *item = ui.tox_friend_list->item(index.row());
    if(!item)
        return;

    QByteArray pkhex = item->data(Qt::UserRole).toByteArray();
    QByteArray pk = QByteArray::fromHex(pkhex);
    QByteArray uid = pk.left(10);

    DwOString duid(uid.constData(), 0, uid.length());
    simple_call *sc = simple_call::get_simple_call(duid);
    sc->setVisible(1);
    sc->raise();
    emit Mainwinform->uid_selected(duid, 4);
}

#endif


/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "adminw.h"
#include "ui_adminw.h"
#include <QIcon>
#include <QHeaderView>
#include <QDateTime>
#include <QMessageBox>
#include "dlli.h"
#include "dwstr.h"
#include "mainwin.h"
#include "userlob.h"

adminw *Adminform;
extern DwOString My_uid;
extern int ClientGod;

extern DWYCO_LIST Model_dir;

DwOString dwyco_get_attr(DWYCO_LIST l, int row, const char *col);

adminw::adminw(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::adminw)
{
    if(Adminform)
        cdcxpanic("too many adminforms");
    Adminform = this;

    ui->setupUi(this);

    QStringList h;
    h << "UID" << "Name" << "Unblock" << "Reason" << "Blocker" << "Name" << "When";
    ui->block_table->setHorizontalHeaderLabels(h);
    h.clear();
    h << "UID" << "Name";
    ui->mod_table->setHorizontalHeaderLabels(h);
    connect(Mainwinform, SIGNAL(admin_event(int)), this, SLOT(admin_update(int)));
    connect(Mainwinform, SIGNAL(add_user_lobby(user_lobby)), this, SLOT(add_user_lobby(user_lobby)));
    connect(Mainwinform, SIGNAL(uid_selected(DwOString, int)), this, SLOT(uid_selected_event(DwOString,int)));
    connect(Mainwinform, SIGNAL(chat_event(int,int,QByteArray,QString,QVariant,int,int)),
            SLOT(chat_event(int,int,QByteArray,QString, QVariant,int,int)));
    setAttribute(Qt::WA_QuitOnClose, 0);
}

adminw::~adminw()
{
    delete ui;
}

static void
resize_table(QTableWidget *tw)
{
    tw->resizeColumnsToContents();
    tw->resizeRowsToContents();
}

void
adminw::clear_block()
{
    ui->block_table->setRowCount(0);
}

void
adminw::clear_mod()
{
    ui->mod_table->setRowCount(0);
}

void
adminw::clear_server_list()
{
    ui->server_table->setRowCount(0);
}

static
QTableWidgetItem *
get_uid_item(DWYCO_LIST v, const char *col, QString& name_out)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(v, 0, col, &val, &len, &type))
        return 0;
    if(type != DWYCO_TYPE_STRING)
        return 0;
    DwOString u(val, 0, len);
    name_out = dwyco_info_to_display(u);
    return new QTableWidgetItem(to_hex(u).c_str());
}

static
QTableWidgetItem *
get_uid_item(QList<QVariant> v, const char *col, QString& name_out)
{
    QByteArray ba = v[atoi(col)].toByteArray();

    DwOString u(ba.constData(), 0, ba.length());
    name_out = dwyco_info_to_display(u);
    return new QTableWidgetItem(to_hex(u).c_str());
}

static
QTableWidgetItem *
get_str_item(DWYCO_LIST v, const char *col)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(v, 0, col, &val, &len, &type))
        return 0;
    if(type != DWYCO_TYPE_STRING)
        return 0;
    return new QTableWidgetItem(QString(QByteArray(val, len)));
}

static
QTableWidgetItem *
get_str_item(QList<QVariant> v, const char *col)
{

    return new QTableWidgetItem(v[atoi(col)].toString());
}

static
QTableWidgetItem *
get_time_item(DWYCO_LIST v, const char *col)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(v, 0, col, &val, &len, &type))
        return 0;
    if(type != DWYCO_TYPE_INT)
        return 0;
    time_t tm = atol(val);
    QDateTime qdt;
    qdt.setSecsSinceEpoch(tm);

    return new QTableWidgetItem(qdt.toString());
}

static
QTableWidgetItem *
get_time_item(QList<QVariant> v, const char *col)
{
    time_t tm = v[atoi(col)].toInt();
    QDateTime qdt;
    qdt.setSecsSinceEpoch(tm);

    return new QTableWidgetItem(qdt.toString());
}

void adminw::chat_event(int cmd,int id, QByteArray uid, QString name, QVariant data, int qid, int extra_arg)
{

    switch(cmd)
    {

    case DWYCO_CHAT_CTX_SYS_ATTR:
        if(name == "icuii-server-rules")
        {
            ui->rules->setHtml(data.toString());

        }
        else if(name == "ui-demigod-list-clear")
            clear_mod();
        else if(name == "ui-block-list-clear")
            clear_block();
        else if(name == "ui-server-list-clear")
            clear_server_list();
        else if(name == "ui-demigod-entry")
        {
            add_mod(data);
        }
        else if(name == "ui-block-entry")
        {
            add_block(data);
        }
#if 0
        else if(name == "ui-server-list-entry")
        {
            add_server(data);
        }
#endif
        else
        {
            add_sys_attr(name, data);
        }

        break;
    case DWYCO_CHAT_CTX_NEW:
        clear_sys_attrs();
        clear_block();
        clear_mod();
        //clear_server_list();
        ui->ulob_table->setRowCount(0);
        break;
    default:
        break;
    }
}

void
adminw::add_block(QVariant vi)
{
    QList<QVariant> v = vi.toList();

    ui->block_table->insertRow(0);
    QString name;
    ui->block_table->setItem(0, 0, get_uid_item(v, DWYCO_BLOCK_REC_BLOCKEE_UID, name));
    ui->block_table->setItem(0, 1, new QTableWidgetItem(name));
    ui->block_table->setItem(0, 2, get_time_item(v, DWYCO_BLOCK_REC_UNBLOCK_TIME));
    ui->block_table->setItem(0, 3, get_str_item(v, DWYCO_BLOCK_REC_REASON));

    ui->block_table->setItem(0, 4, get_uid_item(v, DWYCO_BLOCK_REC_BLOCKER_UID, name));
    ui->block_table->setItem(0, 5, new QTableWidgetItem(name));
    ui->block_table->setItem(0, 6, get_time_item(v, DWYCO_BLOCK_REC_BLOCKED_WHEN));
    resize_table(ui->block_table);
}

void
adminw::add_mod(QVariant v)
{
    QByteArray ba = v.toByteArray();
    DwOString uid(ba.constData(), 0, ba.length());

    QString s = dwyco_info_to_display(uid);
    QTableWidgetItem *inm = new QTableWidgetItem(s);
    QTableWidgetItem *iuid = new QTableWidgetItem(to_hex(uid).c_str());

    ui->mod_table->insertRow(0);
    ui->mod_table->setItem(0, 0, iuid);
    ui->mod_table->setItem(0, 1, inm);
    resize_table(ui->mod_table);

}

void
adminw::clear_sys_attrs()
{
    ui->sys_table->setRowCount(0);
}

void
adminw::add_sys_attr(QString name, QVariant v)
{
    QList<QTableWidgetItem *> li;

    li = ui->sys_table->findItems(name, Qt::MatchExactly);
    QString dum;
    if(li.count() == 0)
    {
        ui->sys_table->insertRow(0);
        ui->sys_table->setItem(0, 0, new QTableWidgetItem(v.typeName()));
        ui->sys_table->setItem(0, 1, new QTableWidgetItem(name));
        ui->sys_table->setItem(0, 2, new QTableWidgetItem(v.toString()));
    }
    else if(li.count() == 1)
    {
        if(li.first()->column() != 1)
            cdcxpanic("fix adminw");
        QTableWidgetItem *wi = ui->sys_table->item(li.first()->row(), 0);
        wi->setText(v.typeName());
        wi = ui->sys_table->item(li.first()->row(), 2);
        wi->setText(v.toString());
    }
    else
    {
        cdcxpanic("oops, fix the adminw");
    }
    resize_table(ui->sys_table);
}

void
adminw::on_set_sys_attr_clicked(bool)
{
    QString name = ui->sys_name->text();
    QString val = ui->sys_val->toPlainText();
    QString tp = ui->type_comboBox->currentText();
    int dt = DWYCO_TYPE_NIL;
    if(tp == "nil")
        dt = DWYCO_TYPE_NIL;
    else if(tp == "int")
        dt = DWYCO_TYPE_INT;
    else if(tp == "str")
        dt = DWYCO_TYPE_STRING;
    dwyco_chat_set_sys_attr(name.toAscii().constData(), name.toAscii().length(),
                            dt, val.toAscii().constData(), val.toAscii().length(), dt == DWYCO_TYPE_INT ? val.toInt() : 0);
}

void
adminw::add_server(QVariant vi)
{
    // i don't think this is ever issued by the server
    // ca 2014, except in emergencies, and in that case,
    // it is handled internally in the dll.
#if 0
    QList<QVariant> v = vi.toList();

    ui->server_table->insertRow(0);
    QString name;
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(v, 0, DWYCO_SL_SERVER_HOSTNAME, &val, &len, &type))
        return;
    ui->server_table->setItem(0, 0, new QTableWidgetItem(QString(QByteArray(val, len))));
    if(!dwyco_list_get(v, 0, DWYCO_SL_SERVER_IP, &val, &len, &type))
        return;
    ui->server_table->setItem(0, 1, new QTableWidgetItem(QString(QByteArray(val, len))));
    if(!dwyco_list_get(v, 0, DWYCO_SL_SERVER_PORT, &val, &len, &type))
        return;
    ui->server_table->setItem(0, 2, new QTableWidgetItem(QString(QByteArray(val, len))));
    if(!dwyco_list_get(v, 0, DWYCO_SL_SERVER_NAME, &val, &len, &type))
        return;
    ui->server_table->setItem(0, 3, new QTableWidgetItem(QString(QByteArray(val, len))));
    if(!dwyco_list_get(v, 0, DWYCO_SL_SERVER_CATEGORY, &val, &len, &type))
        return;
    ui->server_table->setItem(0, 4, new QTableWidgetItem(QString(QByteArray(val, len))));
    resize_table(ui->server_table);
#endif

}
void
adminw::replace_servers(DWYCO_LIST v)
{
    ui->server_table->setRowCount(0);
    int n;
    dwyco_list_numelems(v, &n, 0);
    for(int i = 0; i < n; ++i)
    {
        ui->server_table->insertRow(0);
        QString name;
        const char *val;
        int len;
        int type;
        if(!dwyco_list_get(v, i, DWYCO_SL_SERVER_HOSTNAME, &val, &len, &type))
            return;
        ui->server_table->setItem(0, 0, new QTableWidgetItem(QString(QByteArray(val, len))));
        if(!dwyco_list_get(v, i, DWYCO_SL_SERVER_IP, &val, &len, &type))
            return;
        ui->server_table->setItem(0, 1, new QTableWidgetItem(QString(QByteArray(val, len))));
        if(!dwyco_list_get(v, i, DWYCO_SL_SERVER_PORT, &val, &len, &type))
            return;
        ui->server_table->setItem(0, 2, new QTableWidgetItem(QString(QByteArray(val, len))));
        if(!dwyco_list_get(v, i, DWYCO_SL_SERVER_NAME, &val, &len, &type))
            return;
        ui->server_table->setItem(0, 3, new QTableWidgetItem(QString(QByteArray(val, len))));
        if(!dwyco_list_get(v, i, DWYCO_SL_SERVER_CATEGORY, &val, &len, &type))
            return;
        ui->server_table->setItem(0, 4, new QTableWidgetItem(QString(QByteArray(val, len))));
    }
    resize_table(ui->server_table);

}

void
adminw::refresh_from_data()
{
}

void
adminw::on_actionRefresh_triggered(bool)
{
    dwyco_chat_get_admin_info();
}

void
adminw::on_adminw_activated(int i)
{
    dwyco_chat_get_admin_info();
}

void
adminw::on_block_button_clicked(bool)
{
    int secs;
    secs = ui->minutes_spin->value() * 60 +
           ui->hours_spin->value() * 3600 +
           ui->days_spin->value() * (3600 * 24);

    QByteArray uid = ui->uid_edit->text().toAscii();
    DwOString a(uid, 0, uid.length());
    a = from_hex(a);
    dwyco_chat_set_unblock_time2(0, a.c_str(), a.length(), secs, ui->why_edit->text().toAscii());
    dwyco_chat_get_admin_info();
}

void
adminw::on_unblock_button_clicked(bool)
{
    QByteArray uid = ui->uid_edit->text().toAscii();
    DwOString a(uid, 0, uid.length());
    a = from_hex(a);
    dwyco_chat_set_unblock_time2(0, a.c_str(), a.length(), -1, ui->why_edit->text().toAscii());
    dwyco_chat_get_admin_info();
}

void
adminw::on_mod_button_clicked(bool)
{
    QByteArray uid = ui->uid_edit->text().toAscii();
    DwOString a(uid, 0, uid.length());
    a = from_hex(a);
    dwyco_chat_set_demigod(a.c_str(), a.length(), 1);
    dwyco_chat_get_admin_info();
}

void
adminw::on_unmod_button_clicked(bool)
{
    QByteArray uid = ui->uid_edit->text().toAscii();
    DwOString a(uid, 0, uid.length());
    a = from_hex(a);
    dwyco_chat_set_demigod(a.c_str(), a.length(), 0);
    dwyco_chat_get_admin_info();
}

void
adminw::on_clear_mods_button_clicked(bool)
{
    dwyco_chat_clear_all_demigods();
    dwyco_chat_get_admin_info();
}

void
adminw::add_user_lobby(user_lobby ul)
{
    ui->ulob_table->insertRow(0);
    QString name;
    ui->ulob_table->setItem(0, 0, new QTableWidgetItem(ul.id.c_str()));
    ui->ulob_table->setItem(0, 1, new QTableWidgetItem(ul.hostname.c_str()));
    ui->ulob_table->setItem(0, 2, new QTableWidgetItem(ul.ip.c_str()));
    ui->ulob_table->setItem(0, 3, new QTableWidgetItem(QString::number(ul.port)));
    ui->ulob_table->setItem(0, 4, new QTableWidgetItem(ul.rating.c_str()));
    ui->ulob_table->setItem(0, 5, new QTableWidgetItem(ul.dispname.c_str()));
    ui->ulob_table->setItem(0, 6, new QTableWidgetItem(ul.category.c_str()));
    ui->ulob_table->setItem(0, 7, new QTableWidgetItem(QString::number(ul.max_users)));
    ui->ulob_table->setItem(0, 8, new QTableWidgetItem(to_hex(ul.subgod_uid).c_str()));
}

void
adminw::update_user_lobbies()
{
    ui->ulob_table->setRowCount(0);
    int n = User_lobbies.count();
    for(int i = 0; i < n; ++i)
    {
        add_user_lobby(User_lobbies[i]);
    }
    resize_table(ui->ulob_table);
}


static void
DWYCOCALLCONV
user_lobby_create_callback(const char *cmd, void *arg, int succ, const char *fail)
{
    if(!succ)
    {
        QMessageBox::information(0, "Lobby operation failed", fail);
    }
}

void
adminw::on_create_user_lobby_button_clicked(bool)
{
    dwyco_chat_create_user_lobby(
        ui->dispname_edit->text().toAscii().constData(),
        ui->category_edit->text().toAscii().constData(),
        My_uid.c_str(), My_uid.length(),
        ui->pw_edit->text().toAscii().constData(),
        ui->max_users_edit->text().toInt(),
        user_lobby_create_callback, 0);
}

void
adminw::on_remove_user_lobby_button_clicked(bool)
{
    QList<QTableWidgetItem *> qli;

    qli = ui->ulob_table->selectedItems();
    if(qli.count() == 0)
        return;
    // note: not sure if you have to search for the right
    // column in the list or not
    QString txt = qli[0]->text();
    DwOString a(txt.toAscii().constData());

    dwyco_chat_remove_user_lobby(a.c_str(), user_lobby_create_callback, 0);
}

void
adminw::admin_update(int on)
{
    // if they have the magic stuff, don't mess with the form,
    // as we think they know what htey are doing
    if(ClientGod)
        return;
    if(!on)
    {
        setVisible(0);
        return;
    }
    else if(on == 1)
    {
        ui->tabWidget->setTabEnabled(0, 1);
        ui->tabWidget->setTabEnabled(1, 1);
        ui->tabWidget->setTabEnabled(2, 1);
    }
    else
    {
        ui->tabWidget->setTabEnabled(0, 1);
        ui->tabWidget->setTabEnabled(1, 0);
        ui->tabWidget->setTabEnabled(2, 0);

    }

}

void
adminw::uid_selected_event(DwOString uid, int ctx)
{
    if(uid.length() == 0)
        return;
    ui->uid_edit->setText(to_hex(uid).c_str());
}



void adminw::on_update_rules_clicked()
{
    QString rules = ui->rules->toHtml();
    QByteArray r = rules.toUtf8();
    dwyco_chat_set_sys_attr("icuii-server-rules", 18, DWYCO_TYPE_STRING, r.constData(), r.length(), 0);

}

void adminw::on_rules_textChanged()
{

}

void adminw::on_pushButton_clicked()
{
    dwyco_chat_get_admin_info();
}

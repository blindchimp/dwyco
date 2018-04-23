
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// this model is a one-off read-only model for the user list
// that can be instantiated and used to select users, for example.
#include "ctlist.h"
#include "dlli.h"
#include "getinfo.h"

SimpleContactModel::SimpleContactModel(QObject *parent) :
    QQmlObjectListModel<SimpleContact>(parent, "display")
{
    m_selected_count = 0;
}

SimpleContactModel::~SimpleContactModel()
{
}

static QByteArray
dwyco_get_attr(DWYCO_LIST l, int row, const char *col)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        ::abort();
    if(type != DWYCO_TYPE_STRING && type != DWYCO_TYPE_NIL)
        ::abort();
    return QByteArray(val, len);
}

static int
dwyco_get_attr_int(DWYCO_LIST l, int row, const char *col, int& int_out)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        return 0;
    if(type != DWYCO_TYPE_INT)
        return 0;
    QByteArray str_out = QByteArray(val, len);
    int_out = str_out.toInt();
    return 1;
}


void
SimpleContactModel::set_all_selected(bool b)
{
    int n = count();
    for(int i = 0; i < n; ++i)
    {
        SimpleContact *c = at(i);
        c->set_selected(b);
    }
        
}

void
SimpleContactModel::toggle_selected(int r)
{
    SimpleContact *c = at(r);
    if(!c)
        return;
    c->set_selected(!c->get_selected());
}

void
SimpleContactModel::decr_selected_count(QObject *o)
{
    SimpleContact *c = dynamic_cast<SimpleContact *>(o);
    if(!c)
        return;
    if(c->get_selected())
        update_selected_count(get_selected_count() - 1);
}

void
SimpleContactModel::mod_selected_count(bool newval)
{
    update_selected_count(get_selected_count() + (newval ? 1 : -1));
}

SimpleContact *
SimpleContactModel::add_contact_to_model(const QByteArray& name, const QByteArray& phone, const QByteArray& email)
{

    SimpleContact *c = new SimpleContact(this);
    c->update_display(name);
    c->update_phone(phone);
    c->update_email(email);
    connect(c, SIGNAL(destroyed(QObject*)), this, SLOT(decr_selected_count(QObject*)));
    connect(c, SIGNAL(selectedChanged(bool)), this, SLOT(mod_selected_count(bool)));
    append(c);

    return c;
}


void
SimpleContactModel::load_users_to_model()
{
    DWYCO_LIST l;
    int n;
    clear();
    dwyco_get_contact_list(&l);
    dwyco_list_numelems(l, &n, 0);
    for(int i = 0; i < n; ++i)
    {
        QByteArray name = dwyco_get_attr(l, i, DWYCO_CONTACT_LIST_NAME);
        QByteArray email = dwyco_get_attr(l, i, DWYCO_CONTACT_LIST_EMAIL);
        QByteArray phone = dwyco_get_attr(l, i, DWYCO_CONTACT_LIST_PHONE);

        add_contact_to_model(name, phone, email);

    }
    dwyco_list_release(l);
}

void send_contact_query(QList<QString> emails);

void
SimpleContactModel::send_query()
{
    QList<QString> emails;
    int n = count();
    for(int i = 0; i < n; ++i)
    {
        SimpleContact *c = at(i);

        emails.append(c->get_email());

    }
    send_contact_query(emails);

}



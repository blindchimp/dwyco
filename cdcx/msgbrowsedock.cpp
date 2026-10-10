
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QMessageBox>
#include <QToolBar>
#include "msgbrowsedock.h"
#include "ui_msgbrowsedock.h"
#include "mainwin.h"
#include "composer.h"
#include "dwyco_new_msg.h"

static int
dwyco_get_attr(DWYCO_LIST l, int row, const char *col, DwOString& str_out)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        return 0;
    if(type != DWYCO_TYPE_STRING)
        return 0;
    str_out = DwOString(val, 0, len);
    return 1;
}

MsgBrowseDock::MsgBrowseDock(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::MsgBrowseDock)
{
    ui->setupUi(this);

    QToolBar *tb = new QToolBar(ui->dockWidgetContents);
    tb->addAction(ui->actionReply);
    tb->addAction(ui->actionForward);
    tb->addAction(ui->actionDelete);
    tb->addAction(ui->actionDelete_All);
    ui->verticalLayout->insertWidget(0, tb);

    popup_menu = new QMenu(this);
    popup_menu->addAction(ui->actionForward);
    popup_menu->addAction(ui->actionReply);
    popup_menu->addSeparator();
    popup_menu->addAction(ui->actionDelete);

    connect(Mainwinform, SIGNAL(uid_selected(DwOString,int)), this, SLOT(uid_selected_event(DwOString,int)));
    connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(deferred_load(bool)));
    connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(vis_change(bool)));
    connect(Mainwinform, SIGNAL(ignore_event(DwOString)), this, SLOT(process_ignore_event(DwOString)));
    ui->label->ui.close_button->setVisible(0);
    mm = 0;
    vis = 0;
}

MsgBrowseDock::~MsgBrowseDock()
{
    delete ui;
}

QList<DwOString> *
MsgBrowseDock::get_selection(QModelIndexList *&midxs, int only_one)
{
    QList<DwOString> *ret = new QList<DwOString>;
    QItemSelectionModel *sm = ui->msglist->selectionModel();
    if(!sm)
        return ret;
    QModelIndexList idxs = sm->selectedIndexes();
    midxs = new QModelIndexList;

    if(idxs.count() == 0 || (idxs.count() > 1 && only_one))
        return ret;

    for(int i = 0; i < idxs.count(); ++i)
    {
        if(idxs[i].isValid() && !ui->msglist->isRowHidden(idxs[i].row()))
        {
            QByteArray mid = mm->data(idxs[i], Qt::UserRole).toByteArray();
            if(mid.length() == 0)
                continue;
            ret->append(DwOString(mid.constData(), 0, mid.length()));
            midxs->append(idxs[i]);
        }
    }

    return ret;

}

void
MsgBrowseDock::vis_change(bool a)
{
    vis = a;
    if(vis)
    {
        dwyco_field_debug("browse-old", 1);
    }
}

void
MsgBrowseDock::contextMenuEvent(QContextMenuEvent *ev)
{
    popup_menu->popup(ev->globalPos());
}

void
MsgBrowseDock::uid_selected_event(DwOString uid, int ctx)
{
    if(ctx != 6)
        return;
    if(widget()->isVisible())
    {
        load_model(uid, 1);
        dwyco_field_debug("browse-via-users", 1);
    }
    else
    {
        reset();
        this->uid = uid;
    }
}

void
MsgBrowseDock::process_ignore_event(DwOString uid)
{
    if(this->uid == uid)
    {
        if(dwyco_is_ignored(uid.c_str(), uid.length()))
            reset();
    }
}

void
MsgBrowseDock::deferred_load(bool vis)
{
    if(vis)
    {
        load_model(uid, 1);
    }
}

void
MsgBrowseDock::reset()
{
    ui->msglist->setModel(0);
    ui->label->reset();
    ui->textBrowser->clear();
    if(mm)
        delete mm;
    mm = 0;
    uid = "";
    setWindowTitle("Saved Msgs");
}

int
MsgBrowseDock::load_model(DwOString auid, int init)
{
    DWYCO_MSG_IDX dlt;
    if(!dwyco_get_message_index(&dlt, auid.c_str(), auid.length()))
    {
        reset();
        return 0;
    }
    DWYCO_MSG_IDX dl = dwyco_list_copy(dlt);
    dwyco_list_release(dlt);

    if(mm)
        delete mm;
    mm = new msglist_model(dl);
    mm->uid = auid;
    this->uid = auid;
    ui->msglist->setModel(mm);
    setWindowTitle(dwyco_info_to_display(auid));
    ui->label->reset();
    ui->textBrowser->clear();

    int first_visible = -1;
    if(!ui->show_sent->isChecked())
    {
        // hide the rows for sent msgs
        int n;
        dwyco_list_numelems(dl, &n, 0);
        for(int i = 0; i < n; ++i)
        {
            DwOString str;
            if(dwyco_get_attr(dl, i, DWYCO_MSG_IDX_IS_SENT, str))
            {
                ui->msglist->setRowHidden(i, 1);
            }
            else if(first_visible == -1)
                first_visible = i;
        }
    }
    else
    {
        int n;
        dwyco_list_numelems(dl, &n, 0);
        if(n > 0)
            first_visible = 0;
    }
    // note: it is unclear from the docs just when exactly
    // this selection model becomes valid (ie, non-null). it appears
    // that it is not created until a setModel. we just hope that
    // by doing a setModel, the previous selectionModel is deleted
    // and the connection will be disconnected... but wtf, not sure.

    QItemSelectionModel *ism = ui->msglist->selectionModel();
    if(!ism)
        return 0;
    connect(ism, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(cur_change(QModelIndex,QModelIndex)));
    connect(ism, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(track_multiselect(QItemSelection,QItemSelection)));
    if(first_visible != -1)
        ui->msglist->setCurrentIndex(mm->index(first_visible));
    return 1;
}

void
MsgBrowseDock::track_multiselect(QItemSelection, QItemSelection)
{
    QItemSelectionModel *ism = ui->msglist->selectionModel();
    if(!ism)
        return;
    int multi = ism->selectedRows().count() > 1;
    if(multi)
    {
        ui->actionForward->setEnabled(0);
        ui->actionReply->setEnabled(0);
    }
    else
    {
        ui->actionForward->setEnabled(1);
        ui->actionReply->setEnabled(1);
    }


}

void
MsgBrowseDock::cur_change(const QModelIndex& idx, const QModelIndex& prev)
{
    ui->label->on_stop_button_clicked();
    if(!mm)
        return;

    DWYCO_LIST l = mm->msg_idx;
    if(!l)
        return;

    if(!idx.isValid())
        return;
    int row = idx.row();

    const char *mid;
    int len_mid;
    int type_out;

    if(!dwyco_list_get(l, row, DWYCO_MSG_IDX_MID,
                       &mid, &len_mid, &type_out) || type_out != DWYCO_TYPE_STRING)
        return;
    // i don't think the list_get thing guarantees the result is 0
    // terminated, so just to be sure, do this
    DwOString zmid(mid, 0, len_mid);
    DWYCO_SAVED_MSG_LIST sm;
    switch(dwyco_get_saved_message3(&sm, zmid.c_str()))
    {
    case DWYCO_GSM_ERROR:
        ui->label->hide();
        ui->textBrowser->setText("(failed)");
        return;
    case DWYCO_GSM_TRANSIENT_FAIL:
    case DWYCO_GSM_TRANSIENT_FAIL_AVAILABLE:
        ui->label->hide();
        ui->textBrowser->setText("(finding msg...)");
        return;
    case DWYCO_GSM_PULL_IN_PROGRESS:
        ui->label->hide();
        ui->textBrowser->setText("(fetching...)");
        return;
    default:
        ;
    }
    DWYCO_LIST bt = dwyco_get_body_text(sm);
    DwOString txt;
    if(dwyco_get_attr(bt, 0, DWYCO_NO_COLUMN, txt))
    {
        DwOString att;
        DwOString fn;
        if(dwyco_get_attr(sm, 0, DWYCO_QM_BODY_ATTACHMENT, att))
        {
            ui->label->show();
            ui->label->preview_saved_msg(uid, mid);
        }
        else
        {
            ui->label->hide();
        }

        // this is a security problem, as it allows people to
        // insert html into messages that isn't checked properly.
        // for now, i'm not worried about it.
        //ui->textBrowser->setHtml(get_extended(txt.c_str()));
        ui->textBrowser->clear();
        append_table_to_textedit(sm, ui->textBrowser, "");
        del_unviewed_mid(uid, zmid);
        Mainwinform->decorate_user(uid);
    }
    dwyco_list_release(bt);
    dwyco_list_release(sm);
}

void
MsgBrowseDock::on_actionDelete_triggered(bool)
{
    ui->label->on_stop_button_clicked();

    QModelIndexList *midxs = 0;
    QList<DwOString> *mids = get_selection(midxs, 0);
    if(!midxs)
        return;
    int row = 0;
    for(int i = 0; i < mids->count(); ++i)
    {
        if(i == 0)
            row = (*midxs)[0].row();

        if(!dwyco_delete_saved_message(uid.c_str(), uid.length(), (*mids)[i].c_str()))
            continue;

    }
    ui->label->reset();
    ui->textBrowser->clear();
    delete mids;
    delete midxs;

    load_model(uid);
    // find the next unhidden row and select that
    int n = mm->rowCount();
    if(n == 0)
        return;
    int i = row;
    for(; i < n; ++i)
    {
        if(!ui->msglist->isRowHidden(i))
            break;

    }
    if(i == n)
    {
        row = n - 1;
        if(ui->msglist->isRowHidden(row))
        {
            // scan backwards until we find unhidden row
            // yeesh
            for(i = row; i >= 0; --i)
            {
                if(!ui->msglist->isRowHidden(i))
                {
                    row = i;
                    goto reselect;
                }
            }
            // no unhidden rows
            return;
        }
    }
    else
        row = i;
reselect:
    ;
    QModelIndex mi = ui->msglist->model()->index(row, 0);
    ui->msglist->scrollTo(mi);
    ui->msglist->setCurrentIndex(mi);
}

void
MsgBrowseDock::on_actionForward_triggered(bool)
{
    ui->label->on_stop_button_clicked();
    QItemSelectionModel *ism = ui->msglist->selectionModel();
    if(!ism)
        return;
    if(!ism->hasSelection())
        return;
    QModelIndex mi = ism->currentIndex();
    if(mi.isValid())
    {
        int row = mi.row();
        DWYCO_LIST l = mm->msg_idx;
        if(!l)
            return;
        const char *mid;
        int len_mid;
        int type_out;
        if(!dwyco_list_get(l, row, DWYCO_MSG_IDX_MID,
                           &mid, &len_mid, &type_out) || type_out != DWYCO_TYPE_STRING)
            return;
        // i don't think the list_get thing guarantees the result is 0
        // terminated, so just to be sure, do this
        DwOString zmid(mid, 0, len_mid);

        composer_forward *c = new composer_forward(Mainwinform);
        if(!c->init(QByteArray(uid.c_str(), uid.length()),
                    zmid))
        {
            QMessageBox::information(this, "Can't forward message", "This message can't be forwarded, possibly because it is corrupted on disk.");
            delete c;
            return;
        }
        c->show();
        c->raise();
    }

}

void
MsgBrowseDock::on_actionReply_triggered(bool)
{
    ui->label->on_stop_button_clicked();
    QItemSelectionModel *ism = ui->msglist->selectionModel();
    if(!ism)
        return;
    if(!ism->hasSelection())
        return;
    QModelIndex mi = ism->currentIndex();
    if(mi.isValid())
    {
        composer *c = new composer(COMP_STYLE_REGULAR, 0, this);
        c->set_uid(uid);
        c->show();
        c->raise();
    }
}

void
MsgBrowseDock::on_actionClose_triggered(bool)
{
    ui->label->on_stop_button_clicked();
    if(!close())
        return;
}

void MsgBrowseDock::on_actionDelete_All_triggered()
{
    if(!mm)
        return;
    if(!mm->msg_idx)
        return;
    ui->label->on_stop_button_clicked();
    QMessageBox::StandardButton s = QMessageBox::question(this, "Remove messages",
                                    "Are you sure you want to remove all messages? (NO UNDO)",
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    if(s == QMessageBox::No)
    {
        return;
    }

//    DWYCO_LIST l = mm->msg_idx;
//    if(!l)
//        return;
//    const char *mid;
//    int len_mid;
//    int type_out;
//    int n;
//    dwyco_list_numelems(l, &n, 0);
//    for(int row = 0; row < n; ++row)
//    {
//        if(!dwyco_list_get(l, row, DWYCO_MSG_IDX_MID,
//                           &mid, &len_mid, &type_out) || type_out != DWYCO_TYPE_STRING)
//            continue;
//        // i don't think the list_get thing guarantees the result is 0
//        // terminated, so just to be sure, do this
//        DwOString zmid(mid, 0, len_mid);

//        if(!dwyco_delete_saved_message(uid.c_str(), uid.length(), zmid.c_str()))
//            continue;
//    }
    dwyco_clear_user(uid.c_str(), uid.length());

    // there has got to be a more efficient way to do this, but
    // it is just so convoluted i haven't figured it out yet.
    load_model(uid);
    ui->label->hide();
    ui->textBrowser->setHtml("");
    dwyco_field_debug("delete-all", 1);

}

void MsgBrowseDock::on_show_sent_clicked(bool checked)
{
    ui->label->reset();
    ui->textBrowser->clear();
    msglist_model *m = (msglist_model *)ui->msglist->model();
    if(!m)
        return;
    int n;
    dwyco_list_numelems(m->msg_idx, &n, 0);
    if(!checked)
    {
        for(int i = 0; i < n; ++i)
        {
            DwOString str;
            if(dwyco_get_attr(m->msg_idx, i, DWYCO_MSG_IDX_IS_SENT, str))
            {
                ui->msglist->setRowHidden(i, 1);
            }
        }
    }
    else
    {
        for(int i = 0; i < n; ++i)
        {
            ui->msglist->setRowHidden(i, 0);
        }
    }
    dwyco_field_debug("show-sent", 1);
}

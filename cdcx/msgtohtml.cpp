
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QTextTable>
#include <QTime>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include "dlli.h"
#include "dwstr.h"
#include "msgtohtml.h"
#include "mainwin.h"

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
// this function converts a dwyco message (with possible
// forwarded messages) into a small html table that can be
// inserted into a qt4 textedit as a fragment.
//
//~ table should look something like this
//~ -------------------------
//~ |a/v   | from-user-name |
//~ -------------------------
//~ | time | msg-text       |
//~ -------------------------
//~ if a msg is forwarded, further rows in the
//~ table look like this, with an italic font for
//~ the user-name and other items:
//~ -------------------------
//~ |	   | from-user-name |
//~ -------------------------
//~ | time | msg-text       |
//~ -------------------------

QString
gen_time(DWYCO_SAVED_MSG_LIST l, int row)
{
    int hour;
    int minute;
    int second;

    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, DWYCO_QM_BODY_DATE_HOUR, &val, &len, &type))
        return "";
    if(type != DWYCO_TYPE_INT)
        return "";
    hour = atoi(val);

    if(!dwyco_list_get(l, row, DWYCO_QM_BODY_DATE_MINUTE, &val, &len, &type))
        return "";
    if(type != DWYCO_TYPE_INT)
        return "";
    minute = atoi(val);

    if(!dwyco_list_get(l, row, DWYCO_QM_BODY_DATE_SECOND, &val, &len, &type))
        return "";
    if(type != DWYCO_TYPE_INT)
        return "";
    second = atoi(val);

    QTime qt(hour, minute, second);
    QString t = qt.toString("hh:mm ap");
    return t;
#if 0
    QString ret = QString("<p align=\"right\"><font size=\"6\">") + t + QString("</p>");
    return ret;
#endif

}

static QString
gen_date(DWYCO_SAVED_MSG_LIST l, int row)
{
    time_t seconds;

    const char *val;
    int len;
    int type;

    if(!dwyco_list_get(l, row, DWYCO_QM_BODY_DATE_SECONDS_SINCE_JAN1_1970, &val, &len, &type))
        return "";
    if(type != DWYCO_TYPE_INT)
        return "before 1999";
    seconds = atol(val);

    QDateTime qt = QDateTime::fromTime_t(seconds);
    QString t = qt.toString("M/yyyy");
    return t;

}

// NOTE: send in the top level msg, the zap view function
// will choke if you send it the results of get_body_array
static
int
insert_icons(DWYCO_SAVED_MSG_LIST m, QTextCursor tc)
{
    // insert icons based on the type of msg we received
    DwOString dum;
    if(dwyco_get_attr(m, 0, DWYCO_QM_BODY_FILE_ATTACHMENT, dum))
    {
        // file attachment
        tc.insertImage(":/new/prefix1/downlaod-16x16.png");
        return 1;
    }

    int viewid = dwyco_make_zap_view2(m, 0);
    int audio = 0;
    int video = 0;
    int short_video = 0;

    dwyco_zap_quick_stats_view(viewid, &video, &audio, &short_video);

    dwyco_delete_zap_view(viewid);

    if(short_video)
        tc.insertImage(":/new/prefix1/media 1-16x16.png");
    else
    {
        if(video)
            tc.insertImage(":/new/prefix1/media 2-16x16.png");
        if(audio)
            tc.insertImage(":/new/prefix1/music 2-16x16.png");
    }
    return 1;

}

// this is used when a message is not forwarded, and
// needs a simple format. just
// (a/v) time msg-text
//
static
int
simple_msg_format(DWYCO_LIST ml, QTextTable *tt)
{
    tt->resize(1, 2);

    QString tm = gen_date(ml, 0);

    QTextTableCell cell = tt->cellAt(0, 1);

    QTextCursor tc = cell.firstCursorPosition();
    DwOString att;
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(ml, 0, DWYCO_QM_BODY_ATTACHMENT, &val, &len, &type))
        return 0;
    if(type != DWYCO_TYPE_NIL)
    {
        insert_icons(ml, tc);
    }

    QTextCharFormat ctf;
    ctf.setFontPointSize(6);
    DwOString mid;
    if(!dwyco_get_attr(ml, 0, DWYCO_QM_BODY_ID, mid))
    {
        tc.insertText(tm, ctf);
    }
    else
    {
        QString tmlink = QString("<a href=%1><font size=\"1\">%2</font></a>").arg(mid.c_str(), tm);
        tc.setCharFormat(ctf);
        tc.insertHtml(tmlink);
    }

    DwOString txt;
    if(!dwyco_get_attr(ml, 0, DWYCO_QM_BODY_NEW_TEXT2, txt))
        return 0;

    cell = tt->cellAt(0, 0);
    tc = cell.firstCursorPosition();
    tc.insertFragment(QTextDocumentFragment::fromHtml(get_extended(txt.c_str())));
    QTextTableFormat tf = tt->format();
    tf.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    //tf.setAlignment(Qt::AlignHCenter);
    tf.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    QVector<QTextLength> vt;
    vt.append(QTextLength(QTextLength::PercentageLength, 80));
    vt.append(QTextLength(QTextLength::PercentageLength, 20));
    tf.setColumnWidthConstraints(vt);
    tt->setFormat(tf);
    return 1;
}


int
msg_to_table(DWYCO_SAVED_MSG_LIST m, QTextTable *tt)
{
    DWYCO_LIST ml = dwyco_get_body_array(m);
    if(!ml)
        return 0;
    int msg_comps;
    dwyco_list_numelems(ml, &msg_comps, 0);
    if(msg_comps == 1)
    {
        int ret = simple_msg_format(ml, tt);
        dwyco_list_release(ml);
        return ret;
    }
    tt->resize(msg_comps * 2, 2);
    QTextTableFormat ttf = tt->format();
    ttf.setCellPadding(1);
    ttf.setCellSpacing(0);
    tt->setFormat(ttf);
    for(int i = 0; i < msg_comps; ++i)
    {
        DwOString txt;
        if(!dwyco_get_attr(ml, i, DWYCO_QM_BODY_NEW_TEXT2, txt))
            goto out;
        DwOString uid;
        if(!dwyco_get_attr(ml, i, DWYCO_QM_BODY_FROM, uid))
            goto out;
        QString from_name = dwyco_info_to_display(uid);
        QString tm = gen_date(ml, i);
        tt->mergeCells(i * 2, 0, 1, 2);
        QTextTableCell cell = tt->cellAt(i * 2, 0);
        QTextCursor tc = cell.firstCursorPosition();
        DwOString att;
        const char *val;
        int len;
        int type;
        if(!dwyco_list_get(ml, i, DWYCO_QM_BODY_ATTACHMENT, &val, &len, &type))
            goto out;
        if(type != DWYCO_TYPE_NIL && i == 0)
        {
            insert_icons(m, tc);
        }
        tc = cell.lastCursorPosition();
        if(i < msg_comps - 1)
        {
            from_name = QString("forwarded by: ") + from_name;
        }
        else
        {
            from_name = QString("Created by: ") + from_name;
        }

        if(i >= 1)
        {
            //make forwarded names look italic to
            // make it obvious this isn't from the
            // sender.
            QTextCharFormat tcf = cell.format();
            tcf.setFontItalic(1);
            cell.setFormat(tcf);
        }
        tc.insertText(from_name);

        cell = tt->cellAt(i * 2 + 1, 0);
        tc = cell.firstCursorPosition();
        tc.insertText(tm);

        cell = tt->cellAt(i * 2 + 1, 1);
        tc = cell.firstCursorPosition();
        tc.insertFragment(QTextDocumentFragment::fromHtml(get_extended(txt.c_str())));
    }
    dwyco_list_release(ml);
    return 1;
out:
    dwyco_list_release(ml);
    return 0;
}

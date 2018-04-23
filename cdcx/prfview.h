
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef PRFVIEW_H
#define PRFVIEW_H

#include <QWidget>
#include "dwquerybymember.h"
#include "dwstr.h"
#include "dvp.h"

namespace Ui {
class PrfView;
}

class PrfView : public QWidget
{
    Q_OBJECT

public:
    explicit PrfView(const DwOString &uid, QWidget *parent = 0);
    ~PrfView();
    DVP vp;
    static DwQueryByMember<PrfView> Prf_views;

    QString handle;
    QString desc;
    QString loc;
    QString email;
    DwOString uid;
    int reviewed;
    int regular;
    int viewid;

public:
    bool event(QEvent *event);

    void closeEvent(QCloseEvent *ev);
    int start_fetch();

    void fetch_done(int succ, const char *reason,
                    const char *s1, int len_s1,
                    const char *s2, int len_s2,
                    const char *s3, int len_s3,
                    const char *filename,
                    const char *uid, int len_uid,
                    int reviewed, int regular);


private slots:
    void update_profile(DwOString uid);
    void on_actionPlay_triggered();

    void on_actionStop_triggered();

public:
    Ui::PrfView *ui;
};

#endif // PRFVIEW_H

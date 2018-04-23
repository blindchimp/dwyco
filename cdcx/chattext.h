
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef CHATTEXT_H
#define CHATTEXT_H

#include <QTextBrowser>

class chattext : public QTextBrowser
{
    Q_OBJECT
public:
    explicit chattext(QWidget *parent = 0);

    void resizeEvent(QResizeEvent *e);

signals:

public slots:

};

#endif // CHATTEXT_H

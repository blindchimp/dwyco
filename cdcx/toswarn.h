
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef TOSWARN_H
#define TOSWARN_H

#include <QWidget>

namespace Ui {
class toswarn;
}

class toswarn : public QWidget
{
    Q_OBJECT

public:
    explicit toswarn(QWidget *parent = 0);
    ~toswarn();

private:
    Ui::toswarn *ui;
};

#endif // TOSWARN_H

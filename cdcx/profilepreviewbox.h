
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef PROFILEPREVIEWBOX_H
#define PROFILEPREVIEWBOX_H
#ifdef CDCX_WEBKIT
#include "browsebox.h"
#include "simplehtmlbox.h"

class ProfilePreviewBox : public WHATBOX
{
    Q_OBJECT
public:
    explicit ProfilePreviewBox(QWidget *parent = 0);
    virtual void create_new_url();

signals:

public slots:

};
#endif

#endif // PROFILEPREVIEWBOX_H

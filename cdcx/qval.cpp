
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QRegExpValidator>
#include "qval.h"
static QRegExp EmailRegex("^[a-zA-Z0-9._%+-]+@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,4}$",
                          Qt::CaseSensitive, QRegExp::RegExp2);

static QRegExp SizeRegex("???*", Qt::CaseSensitive, QRegExp::Wildcard);
static QRegExp SubcodeRegex("[a-fA-F0-9]{8,10}", Qt::CaseInsensitive);

QValidator *
email_validator()
{
    QRegExpValidator *v = new QRegExpValidator(EmailRegex, 0);
    return v;
}

QValidator *
size_validator()
{
    QRegExpValidator *v = new QRegExpValidator(SizeRegex, 0);
    return v;
}

QValidator *
subcode_validator()
{
    QRegExpValidator *v = new QRegExpValidator(SubcodeRegex, 0);
    return v;
}

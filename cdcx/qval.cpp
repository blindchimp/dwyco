
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QValidator>
#include "qval.h"
static QRegularExpression EmailRegex("^[a-zA-Z0-9._%+-]+@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,4}$");

//static QRegularExpression SizeRegex("???*");
static QRegularExpression SubcodeRegex("[a-fA-F0-9]{8,10}", QRegularExpression::CaseInsensitiveOption);

QValidator *
email_validator()
{
    QRegularExpressionValidator *v = new QRegularExpressionValidator(EmailRegex, 0);
    return v;
}

QValidator *
size_validator()
{
    QRegularExpressionValidator *v = new QRegularExpressionValidator(QRegularExpression::fromWildcard(QString("???*")), 0);
    return v;
}

QValidator *
subcode_validator()
{
    QRegularExpressionValidator *v = new QRegularExpressionValidator(SubcodeRegex, 0);
    return v;
}

/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "qfiletweak.h"

void cdcxpanic(const char *);

qint64
QFileTweak::readData(char *data, qint64 len)
{
    qint64 nr = QFile::readData(data, len);
    for(int i = 0; i < nr; ++i)
        data[i] ^= 0xff;
    return nr;
}

qint64
QFileTweak::readLineData(char *data, qint64 maxlen)
{
    cdcxpanic("huh");
    return 0;
}

qint64
QFileTweak::writeData(const char *data, qint64 len)
{
    char *a = new char[len];
    for(int i = 0; i < len; ++i)
        a[i] = data[i] ^ 0xff;
    qint64 ret = QFile::writeData(a, len);
    delete [] a;
    return ret;
}

void
QFileTweak::tweak_buf(char *data, int len)
{
    for(int i = 0; i < len; ++i)
        *data++ ^= 0xff;
}

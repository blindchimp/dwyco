
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "qlimitedbuffer.h"

QLimitedBuffer::QLimitedBuffer(int maxsize)
{
    this->maxsize = maxsize;
}

qint64
QLimitedBuffer::writeData(const char * data, qint64 len)
{
    if(pos() + len > maxsize)
        return -1;
    return QBuffer::writeData(data, len);
}

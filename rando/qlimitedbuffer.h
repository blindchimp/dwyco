
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef QLIMITEDBUFFER_H
#define QLIMITEDBUFFER_H

#include <QObject>
#include <QBuffer>

class QLimitedBuffer : public QBuffer
{
public:
    QLimitedBuffer(int maxsize);

private:
    int maxsize;


protected:
    virtual qint64	writeData(const char * data, qint64 len);

};

#endif // QLIMITEDBUFFER_H

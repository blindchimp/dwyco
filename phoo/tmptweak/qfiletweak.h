#ifndef QFILETWEAK_H
#define QFILETWEAK_H
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

// this is just a super-simple file contents tweaker
// that is used to hide our temp image files from
// apps that might scan our data looking for images to
// display. nothing fancy going on here...
//
#include <QFile>
class QFileTweak : public QFile
{
public:
    QFileTweak() {}
    QFileTweak(const QString& fn) : QFile(fn) {}
    QFileTweak(const QString& fn, QObject *p) : QFile(fn, p) {}

    static void tweak_buf(char *buf, int len);

protected:

    virtual qint64 readData(char *data, qint64 len);
    virtual qint64 readLineData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char *data, qint64 len);


};

#endif // QFILETWEAK_H

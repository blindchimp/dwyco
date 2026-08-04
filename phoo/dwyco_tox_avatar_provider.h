
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DWYCOTOXAVATARPROVIDER_H
#define DWYCOTOXAVATARPROVIDER_H

#include <QQuickImageProvider>
#include <QMutex>

class DwycoToxAvatarProvider : public QQuickImageProvider
{
private:
    QMutex mutex;

public:
    DwycoToxAvatarProvider();
    ~DwycoToxAvatarProvider();

    virtual QImage	requestImage(const QString & id, QSize * size, const QSize & requestedSize);

};

#endif // DWYCOTOXAVATARPROVIDER_H

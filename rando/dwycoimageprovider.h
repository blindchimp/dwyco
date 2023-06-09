
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DWYCOIMAGEPROVIDER_H
#define DWYCOIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QMutex>

class DwycoImageProvider : public QQuickImageProvider
{
private:
    QMutex mutex;

public:
    DwycoImageProvider();
    ~DwycoImageProvider();

    void add_image(const QString& id, QImage);

    virtual QImage	requestImage(const QString & id, QSize * size, const QSize & requestedSize);
};

#endif // DWYCOIMAGEPROVIDER_H

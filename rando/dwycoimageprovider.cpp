
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "dwycoimageprovider.h"
#include <QMap>
#include <QMutexLocker>

static QMap<QString, QImage> Dwyco_images;

// TODO: not sure what happens during deletion of one of these if there
// is a thread blocked on the mutex...
DwycoImageProvider::DwycoImageProvider() :
    QQuickImageProvider(QQuickImageProvider::Image)
{

}

void
DwycoImageProvider::add_image(const QString &id, QImage qi)
{
    QMutexLocker lock(&mutex);
    Dwyco_images.insert(id, qi);
}

QImage
DwycoImageProvider::requestImage(const QString & id, QSize * size, const QSize & requestedSize)
{
    // TODO: make reentrant
    QMutexLocker lock(&mutex);

    // note: the image provider really doesn't have a notion of a "one shot"
    // image. i guess it might ask for the same image multiple times, but
    // i'm not sure what the best way to deal with that is.
    QImage ret = Dwyco_images.take(id);
    if(ret.isNull())
        return ret;

    *size = ret.size();
    if(requestedSize.isValid())
        return ret.scaled(requestedSize);
    return ret;
}

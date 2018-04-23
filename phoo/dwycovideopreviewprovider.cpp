
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "dwycovideopreviewprovider.h"

static QImage Dwyco_last_preview;

DwycoVideoPreviewProvider::DwycoVideoPreviewProvider() :
    QQuickImageProvider(QQuickImageProvider::Image)
{

}

void
DwycoVideoPreviewProvider::replace_img(QImage img)
{
    QMutexLocker lock(&mutex);
    Dwyco_last_preview = img;
}

QImage
DwycoVideoPreviewProvider::requestImage(const QString & id, QSize * size, const QSize & requestedSize)
{
    // TODO: make reentrant
    QMutexLocker lock(&mutex);

    QImage ret = Dwyco_last_preview;
    if(ret.isNull())
        return ret;

    *size = ret.size();
    if(requestedSize.isValid())
        return ret.scaled(requestedSize);
    return ret;
}


/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "untweak_provider.h"
#include "qfiletweak.h"
#include <QMutexLocker>
#include <QImageReader>

UntweakProvider::UntweakProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{

}

UntweakProvider::~UntweakProvider()
{

}

QImage
UntweakProvider::requestImage(const QString & id, QSize * size, const QSize & requestedSize)
{
    // TODO: make reentrant
    QMutexLocker lock(&mutex);

    QFileTweak qft(id);
    qft.open(QFile::ReadOnly);
    QImageReader qir(&qft);
    QImage ret = qir.read();

    if(ret.isNull())
        return ret;

    *size = ret.size();
    if(requestedSize.isValid())
        return ret.scaled(requestedSize, Qt::KeepAspectRatio);
    return ret;

}

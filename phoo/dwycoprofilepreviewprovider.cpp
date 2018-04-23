
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "dwycoprofilepreviewprovider.h"
#include <QMutexLocker>
#include "profpv.h"

DwycoProfilePreviewProvider::DwycoProfilePreviewProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
    
}

DwycoProfilePreviewProvider::~DwycoProfilePreviewProvider()
{
    
}

QImage
DwycoProfilePreviewProvider::requestImage(const QString & id, QSize * size, const QSize & requestedSize)
{
    // TODO: make reentrant
    QMutexLocker lock(&mutex);
    
    // id is of format
    // uid (in hex)
    QString nid = id;
    nid.truncate(nid.lastIndexOf("/"));
    QImage ret;
    if(!ThePreviewCache)
        return ret;
    QByteArray buid = nid.toLatin1();
    buid = QByteArray::fromHex(buid);
    
    ret = ThePreviewCache->get_preview_by_uid(buid);
    
    if(ret.isNull())
        return ret;

    *size = ret.size();
    if(requestedSize.isValid())
        return ret.scaled(requestedSize);
    return ret;   

}

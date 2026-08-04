
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "dwyco_tox_avatar_provider.h"
#include <QMutexLocker>
#include "dlli.h"
#include "profpv.h"

DwycoToxAvatarProvider::DwycoToxAvatarProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{

}

DwycoToxAvatarProvider::~DwycoToxAvatarProvider()
{

}

QImage
DwycoToxAvatarProvider::requestImage(const QString & id, QSize * size, const QSize & requestedSize)
{
    // TODO: make reentrant
    QMutexLocker lock(&mutex);

    if(!ThePreviewCache)
        return QImage();

    // id is of format
    // pseudo uid (in hex)
    QString nid = id;
    nid.truncate(nid.lastIndexOf("/"));
    QImage ret;
    QByteArray buid = nid.toLatin1();
    buid = QByteArray::fromHex(buid);

    char *data = 0;
    int len = 0;
    if(dwyco_tox_get_avatar(buid.constData(), buid.length(), &data, &len))
    {
        if(data && len > 0)
            ret.loadFromData((const uchar *)data, len);
        dwyco_free_array(data);
    }

    if(ret.isNull())
        ret = ThePreviewCache->get_default_image();

    *size = ret.size();
    if(requestedSize.isValid())
        return ret.scaled(requestedSize);
    return ret;

}

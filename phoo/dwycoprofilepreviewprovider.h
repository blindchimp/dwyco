
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DWYCOPROFILEPREVIEWPROVIDER_H
#define DWYCOPROFILEPREVIEWPROVIDER_H

#include <QQuickImageProvider>
#include <QMutex>

class DwycoProfilePreviewProvider : public QQuickImageProvider
{
private:
    QMutex mutex;
    
public:
    DwycoProfilePreviewProvider();
    ~DwycoProfilePreviewProvider();
    
    virtual QImage	requestImage(const QString & id, QSize * size, const QSize & requestedSize);
    
    
};

#endif // DWYCOPROFILEPREVIEWPROVIDER_H

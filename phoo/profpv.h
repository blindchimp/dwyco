
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef PROFPV_H
#define PROFPV_H
#include <QObject>
#include <QPixmap>
#include "dwstr.h"

class profpv : public QObject
{
    Q_OBJECT

public:
	profpv();

	QImage get_preview_by_uid(const QByteArray& uid);
    QImage get_default_image();
	void refresh_profile(const QByteArray& uid);

public slots:
	void clear_cache(int);
	void remove_entry(QByteArray);
    // the uid arg in this case is assumed to be hex encoded
    void remove_entry(QString);
    void chat_server(int,int);

signals:
    void profile_preview_resolved(QString);
};

extern profpv *ThePreviewCache;
#endif

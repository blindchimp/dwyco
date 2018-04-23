
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#if 0
class metafilter : public QObject
{
public:
    metafilter(QObject * = 0) {}


    bool eventFilter(QObject *obj, QEvent *e) {
        if(e->type() == QEvent::MetaCall)
        {
            printf("META CALL\n");
        }
        return QObject::eventFilter(obj, e);
    }
};
#endif

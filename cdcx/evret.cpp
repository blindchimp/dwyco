
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QEvent>
#include <QKeyEvent>
#include <QInputMethodEvent>
#include "evret.h"
#include "dlli.h"
#include "simple_call.h"

ReturnFilter::ReturnFilter(QObject *p) : QObject(p) {}

bool
ReturnFilter::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        dwyco_add_entropy_timer((char *)keyEvent, sizeof(*keyEvent));
        //qDebug("Ate key press %d", keyEvent->key());
        emit chat_typing();
        if(keyEvent->key() == Qt::Key_Return)
        {
            emit return_hit();
            return 1;
        }
        if(keyEvent->key() == Qt::Key_Escape)
        {
            emit esc_hit();
            return 1;
        }
        return 0;
    }
    if(e->type() == QEvent::InputMethod)
    {
        emit chat_typing();
        return QObject::eventFilter(obj, e);
    }
#if 0
    else if(e->type() == QEvent::FocusIn || e->type() == QEvent::FocusOut) {
        QFocusEvent *fe = static_cast<QFocusEvent *>(e);
        QObject *a = obj->parent();
        while(a && !dynamic_cast<chatform2 *>(a))
            a = a->parent();
        if(!a)
            return 0;
        chatform2 *b = dynamic_cast<chatform2 *>(a);
        if(e->type() == QEvent::FocusIn)
            b->focusInEvent(fe);
        else
            b->focusOutEvent(fe);
        return 0;
    }
#endif
    else {
        // standard event processing
        return QObject::eventFilter(obj, e);
    }

}

bool
ResizeFilter::eventFilter(QObject *obj, QEvent *e)
{
    if(e->type() == QEvent::Resize)
    {
        emit resized();
    }
    return QObject::eventFilter(obj, e);
}

bool
LocationFilter::eventFilter(QObject *obj, QEvent *e)
{
    return QObject::eventFilter(obj, e);
#if 0
    // the idea was to get the mouse position when a user
    // clicked on a link in a text browser, but those mouse
    // events don't even make it here for some reason when this
    // filter is installed on the QTextBrowser.
    // turns out it is much easier to just use QCursor::pos()
    // when the URL signal is emitted.
    if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        dwyco_add_entropy_timer((char *)me, sizeof(*me));
        QObject *a = obj->parent();
        while(a && !dynamic_cast<chatform2 *>(a))
            a = a->parent();
        if(!a)
            return 0;
        chatform2 *b = dynamic_cast<chatform2 *>(a);
        b->mouseReleaseEvent(me);
        return 0;
    }
    else
    {
        // standard event processing
        return QObject::eventFilter(obj, e);
    }
#endif


}

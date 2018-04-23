
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef QMSG2_H
#define QMSG2_H
// special message structure
//
// special messages are messages that
// contain something other than the usual
// text to be displayed to a user.
// they are sent/received in the normal
// manner as other messages, but they
// can contain extra information that
// clients can interpret in their own
// way. this allows clients to
// come up with thier own protocols outside
// the usual ones supported by the core.
// this also allows clients to be incompatible (sigh).
// it is crucial that clients ignore types they
// don't understand.
//
// special messages contain all the usual message
// fields, and they will be identical to a normal
// message in terms of what the fields mean (like the
// authentication, etc.) special messages cannot be
// forwarded at the moment.
//
// special messages have a field that includes
// all the items needed to process the message
// in this form:
// vector(type vector(type specific items))
//
// this special vector is inserted at a known location
// in a normal message.
// current uses of the special message structure are
// for processing pal authentication messages.
//
//

// in the comment A is the requester
// B is the recipient
#define QM_SP_PAL_AUTH_REQ "palreq"
#define		QM_PALREQ_PREAUTH 0		// added by server on request, roughly SHA(A, B, secret)
// the server computes this and sends it to
// the recipient in the request. if the
// recipient grants the request, they send it
// back to the requester.
#define		QM_PALREQ_RECIP_GRANTS_REQ 1 // added by server , roughly SHA(B, A, secret)

#define QM_SP_PAL_AUTH_OK "palok"
#define		QM_PALOK_COOKIE 0		// added by server roughly SHA(B, A, secret)

#define QM_SP_PAL_AUTH_REJ "palrej"

#define QM_SP_PAL_AUTH_REVOKE "palrev"

#define QM_SP_USER_DEFINED "user"

#define QM_SP_ADMIN "admin"

#endif

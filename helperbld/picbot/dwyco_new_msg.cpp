/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
// call this function on "rescan msgs" and it will
// return the list of new msgs received
//

#include <stdio.h>
#include "dwyco_new_msg.h"
#include "dlli.h"
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <QSet>
#include <QByteArray>
#include <QList>

static QList<QByteArray> fetching;
static QSet<QByteArray> Dont_refetch;
static QList<QByteArray> Delete_msgs;
static QSet<QByteArray> Already_returned;

static int
dwyco_get_attr(DWYCO_LIST l, int row, const char *col, QByteArray& str_out)
{
	const char *val;
	int len;
	int type;
	if(!dwyco_list_get(l, row, col, &val, &len, &type))
		return 0;
	if(type != DWYCO_TYPE_STRING)
		return 0;
    str_out = QByteArray(val, len);
	return 1;
}

static int
dwyco_test_attr(DWYCO_LIST l, int row, const char *col)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        return 0;
    if(type == DWYCO_TYPE_NIL)
        return 0;

    return 1;
}

void
DWYCOCALLCONV
msg_callback(int id, int what, const char *mid, void *)
{
//printf("id %d what %d mid %s\n", id, what, mid);
	int i = fetching.indexOf(mid);
	switch(what)
	{
	case DWYCO_MSG_DOWNLOAD_FETCHING_ATTACHMENT:
        // here is a case where it makes sense to provide the text of the message
        // immediately while the attachment is being downloaded in the background.
        return;

    case DWYCO_MSG_DOWNLOAD_RATHOLED:
        // if this msg is ratholed, it can never be fetched, so just delete it.
        Dont_refetch.insert(mid);
        if(i >= 0)
            fetching.removeAt(i);
        Delete_msgs.append(mid);
        return;

	case DWYCO_MSG_DOWNLOAD_ATTACHMENT_FETCH_FAILED:
	case DWYCO_MSG_DOWNLOAD_SAVE_FAILED:
	case DWYCO_MSG_DOWNLOAD_FAILED:
        // this is potentially just a transient failure, so maybe try refetching
        // a certain number of times. really need something from the server that says
        // whether there is any hope of getting the message or not.
        // i think i will handle this case on the server and just delete messages that
        // have not been fetched in a month or something. this will cause a bit of thrashing for
        // users that have a lot of messages that can't be fetched, but gives the best chance to get
        // a message in transient failure situations.
        Dont_refetch.insert(mid);
		if(i >= 0)
			fetching.removeAt(i);
		return;
	case DWYCO_MSG_DOWNLOAD_OK:
	default:
		if(i >= 0)
			fetching.removeAt(i);
	}
}

int
dwyco_new_msg(QByteArray& uid_out, QByteArray& txt, int& zap_viewer, QByteArray& mid, int& has_att)
{
	zap_viewer = 0;

    int k = Delete_msgs.count();
    for(int i = 0; i < k; ++i)
    {
        dwyco_delete_unsaved_message(Delete_msgs[i].constData());

    }
    Delete_msgs.clear();

	DWYCO_UNSAVED_MSG_LIST ml;
	if(!dwyco_get_unsaved_messages(&ml, 0, 0))
		return 0;
	int n;
	const char *val;
	int len;
	int type;
	dwyco_list_numelems(ml, &n, 0);
	if(n == 0)
    {
        dwyco_list_release(ml);
		return 0;
    }
	for(int i = 0; i < n; ++i)
	{
		if(!dwyco_get_attr(ml, i, DWYCO_QMS_FROM, uid_out))
			continue;
		if(!dwyco_get_attr(ml, i, DWYCO_QMS_ID, mid))
			continue;
        if(Already_returned.contains(mid))
            continue;
		if(Dont_refetch.contains(mid) ||
            fetching.contains(mid) || fetching.count() >= 5)
			continue;
		if(!dwyco_list_get(ml, i, DWYCO_QMS_IS_DIRECT, &val, &len, &type))
			continue;
		if(type == DWYCO_TYPE_NIL)
        {
            int special_type;
            const char *uid;
            int len_uid;
            const char *dlv_mid;
            if(dwyco_is_delivery_report(mid.constData(), &uid, &len_uid, &dlv_mid, &special_type))
            {
                // process pal authorization stuff here
                if(special_type == DWYCO_SUMMARY_DELIVERED)
                {
                    // hmmm, need new api to get uid/mid of delivered msg
                    dwyco_delete_unsaved_message(mid.constData());
                    continue;
                }

            }
			// issue a server fetch, client will have to
			// come back in to get it when the fetch is done
			fetching.append(mid);
			dwyco_fetch_server_message(mid.constData(), msg_callback, 0, 0, 0);
			continue;
		}

#if 0
		if(dwyco_is_special_message(0, 0, mid.constData(), &special_type))
		{
			// process pal authorization stuff here
			switch(special_type)
			{
            case DWYCO_SUMMARY_DELIVERED:
                // hmmm, need new api to get uid/mid of delivered msg
                dwyco_delete_unsaved_message(mid.constData());
                break;

#if 0
			case DWYCO_PAL_AUTH_REQ:
				// note that most of the "special message" stuff only
				// works on unsaved messages. which is a pain.
				display_msg(uid_out, "Pal authorization request", 0, QByteArray(""));
				// yuck, this chat_uids stuff needs to be encapsulated
				{
				int i = chat_uids.index(uid_out);
				if(i == -1)
					break;
				chatform2 *c = chat_wins[i];
				dwyco_get_unsaved_message(&c->pal_auth_req_msg, mid.constData());
				// note: we can't delete it if it has an attachment...
				// we can *save* it ok, but the unsaved msg's attachment
				// will become unreadable, but since we don't need the
				// attachment for pal auth purposes, we'll ignore this "bug"
				}
				goto save_it;
				break;
			case DWYCO_PAL_OK:
				dwyco_handle_pal_auth(0, 0, mid.constData(), 1);
				display_msg(uid_out, "Pal authorization accepted", 0, QByteArray(""));
				dwyco_delete_unsaved_message(mid.constData());
				break;
			case DWYCO_PAL_REJECT:
				dwyco_handle_pal_auth(0, 0, mid.constData(), 1);
				dwyco_pal_delete(uid_out.constData(), uid_out.length());
				display_msg(uid_out, "Pal authorization rejected", 0, QByteArray(""));
				dwyco_delete_unsaved_message(mid.constData());
				break;
#endif
			default:
				dwyco_delete_unsaved_message(mid.constData());
				break;// do nothing, ignore it.
			}
			continue;
		}
#endif
		DWYCO_SAVED_MSG_LIST sm;
		if(!dwyco_unsaved_message_to_body(&sm, mid.constData()))
			continue;
		DWYCO_LIST bt = dwyco_get_body_text(sm);
		if(!dwyco_get_attr(bt, 0, DWYCO_NO_COLUMN, txt))
			continue;

        has_att = dwyco_test_attr(sm, 0, DWYCO_QM_BODY_ATTACHMENT);

		dwyco_list_release(bt);
		dwyco_list_release(sm);
		dwyco_list_release(ml);
        Already_returned.insert(mid);
		return 1;
	}
    dwyco_list_release(ml);
	return 0;
}

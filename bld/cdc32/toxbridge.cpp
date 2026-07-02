#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <map>

#include "toxbridge.h"
#include "../../toxd/toxd_plugin.h"
#include "qauth.h"
#include "dwrtlog.h"
#include "se.h"
#include "qmsg.h"
#include "ser.h"
#include "simplesql.h"
#include "xinfo.h"
#include "fnmod.h"
#include "qdirth.h"
#include "qmsgsql.h"

// toxcore error codes — only the ones we check on the bridge side
#define TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_CONNECTED 3

namespace dwyco {

static ToxPlugin *Tox_plugin;
static int Started;
static ToxQueue *Tox_q;
static vc Friend_cache;
struct IncomingFileTransfer {
    DwString tmp_basename;
    int fd;
    uint64_t expected_size;
    uint64_t received_size;
    DwString original_filename;
    vc pseudo_uid;
};
static std::map<DwString, IncomingFileTransfer> Incoming_xfers;

struct OutgoingFileTransfer {
    DwString file_path;
    int fd;
    uint64_t file_size;
    uint64_t sent_size;
    DwString original_filename;
    int64_t row_id;
    vc local_mid;
    uint32_t tox_fn;
    uint32_t tox_fnum;
};
static std::map<DwString, OutgoingFileTransfer> Outgoing_xfers;

static void
fail_outgoing(const vc &local_mid)
{
    if(!Tox_q)
        return;
    Tox_q->sql_simple("UPDATE tox_outbox SET status=3 WHERE local_mid=?1", local_mid);
    vc res = Tox_q->sql_simple("SELECT recipient_pseudo FROM tox_outbox WHERE local_mid=?1", local_mid);
    vc recip;
    if(!res.is_nil() && res.num_elems() > 0)
        recip = res[0][0];
    se_emit_msg(SE_MSG_SEND_FAIL, local_mid, recip);
}

static DwString
xfer_key(uint32_t fn, uint32_t fnum)
{
    return DwString::fromInt(fn) + "_" + DwString::fromInt(fnum);
}

static void
safe_add_crdt_tag(const vc &mid, const vc &tag)
{
    vc existing = sql_get_tagged_mids2(tag);
    for(int i = 0; i < existing.num_elems(); ++i)
    {
        if(existing[i][0] == mid)
            return;
    }
    sql_add_tag(mid, tag);
}

vc
tox_pubkey_to_pseudo_uid(const vc &pubkey)
{
    if(pubkey.len() < 10)
        return vcnil;
    return vc(VC_BSTRING, (const char *)pubkey, 10);
}

vc
make_tox_info_vec(const vc &pseudo, const vc &name)
{
    vc v(VC_VECTOR);
    v[QIR_FROM] = pseudo;
    v[QIR_HANDLE] = name;
    v[QIR_EMAIL] = "";
    v[QIR_USER_SPECED_ID] = "";
    v[QIR_FIRST] = "";
    v[QIR_LAST] = "";
    v[QIR_DESCRIPTION] = "";
    v[QIR_LOCATION] = "";
    return v;
}

static vc
vcblob(const vc &v)
{
    vc ret(VC_VECTOR);
    ret.append(vc("blob"));
    ret.append(v);
    return ret;
}

static vc
make_msg_body(const vc &from, const vc &text,
              const vc &attachment = vcnil,
              const vc &file_attachment = vcnil,
              const vc &estimated_size = vcnil)
{
    vc body(VC_VECTOR);
    body[QQM_BODY_FROM] = from;
    body[QQM_BODY_TEXT_do_not_use] = vcnil;
    body[QQM_BODY_ATTACHMENT] = attachment;
    body[QQM_BODY_DATE] = date_vector();
    body[QQM_BODY_RATING_do_not_use] = vcnil;
    body[QQM_BODY_AUTH_VEC] = vcnil;
    body[QQM_BODY_FORWARDED_BODY] = vcnil;
    body[QQM_BODY_NEW_TEXT] = text;
    body[QQM_BODY_ATTACHMENT_LOCATION] = vcnil;
    body[QQM_BODY_SPECIAL_TYPE] = vcnil;
    body[QQM_BODY_NO_FORWARD] = vcnil;
    body[QQM_BODY_FILE_ATTACHMENT] = file_attachment;
    body[QQM_BODY_DHSF] = vcnil;
    body[QQM_BODY_EMSG] = vcnil;
    body[QQM_BODY_ESTIMATED_SIZE] = estimated_size;
    body[QQM_BODY_NO_DELIVERY_REPORT] = vcnil;
    body[QQM_BODY_LOGICAL_CLOCK] = (int64_t)++Logical_clock;
    body[QQM_BODY_FROM_GROUP] = vcnil;
    return body;
}

static void
tox_uid_cache_add(const vc &pseudo_uid)
{
    if(Tox_q)
        Tox_q->sql_simple("INSERT OR IGNORE INTO tox_uid_type VALUES(?1)", vcblob(pseudo_uid));
}

static void
process_tox_event(const char *type, const vc &args)
{

    if(strcmp(type, "friend_request") == 0 && args.num_elems() >= 2) {
        vc pubkey = args[0];
        vc msg = args[1];
        vc pseudo = tox_pubkey_to_pseudo_uid(pubkey);
        tox_uid_cache_add(pseudo);
        GRTLOG("tox: friend request from ", 0, 0);
        GRTLOGVC(pseudo);
        se_emit(SE_TOX_FRIEND_REQUEST, pseudo, msg, pubkey);

    } else if(strcmp(type, "message") == 0 && args.num_elems() >= 4) {
        uint32_t fn = (uint32_t)(int)args[0];
        vc pubkey = args[1];
        vc text = args[2];
        vc msg_type = args[3];
        vc pseudo = tox_pubkey_to_pseudo_uid(pubkey);
        tox_uid_cache_add(pseudo);

        GRTLOG("tox: message from fn %d", (int)fn, 0);
        GRTLOGVC(pseudo);

        vc body = make_msg_body(pseudo, text);

        vc qqm(VC_VECTOR);
        qqm[QQM_RECIP_VEC] = vc(VC_VECTOR);
        qqm[QQM_RECIP_VEC][0] = My_UID;
        qqm[QQM_MSG_VEC] = body;
        qqm[QQM_LOCAL_ID] = to_hex(gen_id());

        store_direct(0, qqm, 0);

        se_emit(SE_TOX_MESSAGE, pseudo);

    } else if(strcmp(type, "read_receipt") == 0 && args.num_elems() >= 2) {
        uint32_t fn = (uint32_t)(int)args[0];
        uint32_t mid = (uint32_t)(int)args[1];
        GRTLOG("tox: read receipt for mid %d", (int)mid, 0);
        if(Tox_q)
        {
            vc pseudo;
            if(!tox_friend_number_to_pseudo_uid(fn, pseudo))
                pseudo = vcnil;
            if(!pseudo.is_nil())
                pseudo = vcblob(pseudo);
            vc res = Tox_q->sql_simple(
                "SELECT id, local_mid, qqm_blob FROM tox_outbox "
                "WHERE recipient_pseudo=?1 AND tox_mid=?2 AND status=1",
                pseudo, vc((int)mid));
            if(!res.is_nil() && res.num_elems() > 0)
            {
                vc row = res[0];
                int64_t row_id = (int64_t)(long long)row[0];
                vc local_mid = row[1];
                vc qqm_blob = row[2];
                vc qqm;
                if(deserialize(qqm_blob, qqm) > 0)
                {
                    DwString tmpfn = gen_random_filename() + ".q";
                    if(save_info(qqm, tmpfn))
                    {
                        do_local_store(tmpfn, local_mid);
                        unlink(newfn(tmpfn).c_str());
                    }
                }
                Tox_q->mark_sent(row_id);
            }
        }
        vc recip;
        if(tox_friend_number_to_pseudo_uid(fn, recip))
            se_emit(SE_TOX_READ_RECEIPT, recip, vc((int)mid));

    } else if(strcmp(type, "friend_connection_status") == 0 && args.num_elems() >= 3) {
        uint32_t fn = (uint32_t)(int)args[0];
        vc pubkey = args[1];
        vc status = args[2];
        vc pseudo = tox_pubkey_to_pseudo_uid(pubkey);
        tox_uid_cache_add(pseudo);
        GRTLOG("tox: friend %d online=%d", (int)fn, status == vc("online") ? 1 : 0);
        se_emit(SE_TOX_FRIEND_STATUS, pseudo, status);

    } else if(strcmp(type, "friend_name") == 0 && args.num_elems() >= 3) {
        uint32_t fn = (uint32_t)(int)args[0];
        vc pubkey = args[1];
        vc name = args[2];
        vc pseudo = tox_pubkey_to_pseudo_uid(pubkey);
        tox_uid_cache_add(pseudo);
        (void)fn;
        se_emit(SE_TOX_FRIEND_NAME, pseudo, name);
        if(!name.is_nil() && name != vc(""))
        {
            vc old;
            int same = Session_infos.find(pseudo, old) && old.num_elems() > QIR_HANDLE && old[QIR_HANDLE] == name;
            Session_infos.add_kv(pseudo, make_tox_info_vec(pseudo, name));
            if(!same)
                save_info(Session_infos, "sinfo");
            DwString composite;
            composite += to_hex(pseudo);
            composite += "_";
            composite += to_hex(name);
            safe_add_crdt_tag(composite, "_tox_friend");
        }

    } else if(strcmp(type, "file_request") == 0 && args.num_elems() >= 5) {
        uint32_t fn = (uint32_t)(int)args[0];
        uint32_t fnum = (uint32_t)(int)args[1];
        uint32_t kind = (uint32_t)(int)args[2];
        uint64_t size = 0;
        if(args[3].type() == VC_INT)
            size = (uint64_t)(long long)args[3];
        vc name_vc = args[4];
        DwString original_name((const char *)name_vc, name_vc.len());

        vc pseudo;
        if(!tox_friend_number_to_pseudo_uid(fn, pseudo))
            return;

        if(kind == 1)
        {
            GRTLOG("tox: rejecting avatar transfer fn=%d fnum=%d", fn, fnum);
            toxp_file_cancel(Tox_plugin, fn, fnum);
            return;
        }

        int err = 0;
        if(!toxp_file_accept(Tox_plugin, fn, fnum, &err))
        {
            GRTLOG("tox: auto-accept failed fn=%d fnum=%d", fn, fnum);
            GRTLOG("tox: auto-accept err=%d", err, 0);
            return;
        }

        DwString tmp_basename = gen_random_filename() + ".tmp";
        DwString tmp_path = newfn(tmp_basename);
        int fd = open(tmp_path.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0600);
        if(fd == -1)
        {
            GRTLOG("tox: cant open temp file %s", tmp_path.c_str(), 0);
            return;
        }

        IncomingFileTransfer xfer;
        xfer.tmp_basename = tmp_basename;
        xfer.fd = fd;
        xfer.expected_size = size;
        xfer.received_size = 0;
        xfer.original_filename = dwbasename(original_name);
        xfer.pseudo_uid = pseudo;
        Incoming_xfers[xfer_key(fn, fnum)] = xfer;

        GRTLOG("tox: file recv accepted fn=%d fnum=%d", fn, fnum);

    } else if(strcmp(type, "self_connection_status") == 0 && args.num_elems() >= 1) {
        vc status = args[0];
        GRTLOG("tox: self connection status %s", (const char *)status, 0);

        se_emit(SE_TOX_SELF_CONNECTION_STATUS, My_UID, status);

    } else if(strcmp(type, "file_chunk") == 0 && args.num_elems() >= 4) {
        uint32_t fn = (uint32_t)(int)args[0];
        uint32_t fnum = (uint32_t)(int)args[1];
        uint64_t pos = 0;
        if(args[2].type() == VC_INT)
            pos = (uint64_t)(long long)args[2];
        vc data = args[3];
        (void)pos;

        DwString key = xfer_key(fn, fnum);
        std::map<DwString, IncomingFileTransfer>::iterator it = Incoming_xfers.find(key);
        if(it == Incoming_xfers.end())
            return;
        IncomingFileTransfer &xfer = it->second;

        // nil data signals EOF → file complete
        if(data.is_nil())
        {
            close(xfer.fd);
            xfer.fd = -1;

            DwString fle_basename = gen_random_filename() + ".fle";
            DwString src = newfn(xfer.tmp_basename);
            DwString dst = newfn(fle_basename);
            if(CopyFile(src.c_str(), dst.c_str(), 0))
            {
                vc body = make_msg_body(xfer.pseudo_uid, vc(""),
                                        fle_basename,
                                        xfer.original_filename,
                                        vc((long long)xfer.expected_size));

                vc qqm(VC_VECTOR);
                qqm[QQM_RECIP_VEC] = vc(VC_VECTOR);
                qqm[QQM_RECIP_VEC][0] = My_UID;
                qqm[QQM_MSG_VEC] = body;
                qqm[QQM_LOCAL_ID] = to_hex(gen_id());

                store_direct(0, qqm, 0);
                se_emit(SE_TOX_MESSAGE, xfer.pseudo_uid);
                GRTLOG("tox: file recv complete fn=%d fnum=%d", fn, fnum);
            }
            else
            {
                GRTLOG("tox: failed to copy completed file %s", src.c_str(), 0);
            }
            DeleteFile(src.c_str());
            Incoming_xfers.erase(it);
        }
        else
        {
            // normal data chunk — append to temp file
            const char *buf = (const char *)data;
            size_t len = data.len();
            ssize_t n = write(xfer.fd, buf, len);
            if(n < 0 || (size_t)n != len)
            {
                GRTLOG("tox: file write error fn=%d fnum=%d", fn, fnum);
                close(xfer.fd);
                DeleteFile(newfn(xfer.tmp_basename).c_str());
                Incoming_xfers.erase(it);
            }
            else
            {
                xfer.received_size += len;
            }
        }

    } else if(strcmp(type, "file_chunk_request") == 0 && args.num_elems() >= 4) {
        uint32_t fn = (uint32_t)(int)args[0];
        uint32_t fnum = (uint32_t)(int)args[1];
        uint64_t position = 0;
        if(args[2].type() == VC_INT)
            position = (uint64_t)(long long)args[2];
        uint64_t length = 0;
        if(args[3].type() == VC_INT)
            length = (uint64_t)(long long)args[3];

        DwString key = xfer_key(fn, fnum);
        std::map<DwString, OutgoingFileTransfer>::iterator oit = Outgoing_xfers.find(key);
        if(oit == Outgoing_xfers.end())
            return;
        OutgoingFileTransfer &xfer = oit->second;

        // length == 0 signals toxcore has confirmed transfer complete
        if(length == 0)
        {
            close(xfer.fd);
            GRTLOG("tox: file send complete fn=%d fnum=%d", (int)fn, (int)fnum);

            if(Tox_q)
            {
                vc qqm_blob = Tox_q->load_qqm_blob(xfer.row_id);
                if(!qqm_blob.is_nil())
                {
                    vc qqm;
                    if(deserialize(qqm_blob, qqm) > 0)
                    {
                        DwString tmpfn = gen_random_filename() + ".q";
                        if(save_info(qqm, tmpfn))
                        {
                                do_local_store(tmpfn, xfer.local_mid);
                                DeleteFile(newfn(tmpfn).c_str());
                        }
                    }
                }
                Tox_q->mark_sent(xfer.row_id);
            }

            Outgoing_xfers.erase(oit);
            return;
        }

        // read length bytes at position and send
        if(lseek(xfer.fd, position, SEEK_SET) < 0)
        {
            GRTLOG("tox: file_chunk_request lseek error fn=%d fnum=%d", (int)fn, (int)fnum);
            return;
        }

        char buf[4096];
        if(length > sizeof(buf))
            length = sizeof(buf);
        ssize_t n = read(xfer.fd, buf, (size_t)length);
        if(n < 0 || (size_t)n != length)
        {
            GRTLOG("tox: file_chunk_request read error fn=%d fnum=%d", (int)fn, (int)fnum);
            return;
        }

        vc chunk(VC_BSTRING, buf, n);
        if(toxp_file_send_data(Tox_plugin, fn, fnum, position, chunk))
        {
            xfer.sent_size += n;
        }
        else
        {
            GRTLOG("tox: file_chunk_request send error fn=%d fnum=%d", (int)fn, (int)fnum);
        }

    } else if(strcmp(type, "file_control") == 0 && args.num_elems() >= 3) {
        uint32_t fn = (uint32_t)(int)args[0];
        uint32_t fnum = (uint32_t)(int)args[1];
        vc control = args[2];
        DwString key = xfer_key(fn, fnum);
        std::map<DwString, IncomingFileTransfer>::iterator it = Incoming_xfers.find(key);
        if(it != Incoming_xfers.end())
        {
            if(control == vc("cancel"))
            {
                if(it->second.fd >= 0)
                    close(it->second.fd);
                DeleteFile(newfn(it->second.tmp_basename).c_str());
                Incoming_xfers.erase(it);
                GRTLOG("tox: xfer cancelled by sender fn=%d fnum=%d", fn, fnum);
            }
            return;
        }
        std::map<DwString, OutgoingFileTransfer>::iterator oit = Outgoing_xfers.find(key);
        if(oit != Outgoing_xfers.end())
        {
            if(control == vc("resume"))
            {
                oit->second.fd = open(oit->second.file_path.c_str(), O_RDONLY);
                if(oit->second.fd < 0)
                {
                    GRTLOG("tox: outgoing xfer resume, cant open %s", oit->second.file_path.c_str(), 0);
                    toxp_file_cancel(Tox_plugin, fn, fnum);
                    fail_outgoing(oit->second.local_mid);
                    Outgoing_xfers.erase(oit);
                }
                else
                {
                    GRTLOG("tox: outgoing xfer resume fn=%d fnum=%d", fn, fnum);
                }
            }
            else if(control == vc("cancel"))
            {
                GRTLOG("tox: outgoing xfer cancelled by receiver fn=%d fnum=%d", fn, fnum);
                if(oit->second.fd >= 0)
                    close(oit->second.fd);
                fail_outgoing(oit->second.local_mid);
                Outgoing_xfers.erase(oit);
            }
            return;
        }

    } else if(strcmp(type, "ready") == 0) {
        GRTLOG("tox: ready", 0, 0);
        se_emit(SE_TOX_READY, vcnil);

    } else if(strcmp(type, "typing") == 0 && args.num_elems() >= 3) {
        uint32_t fn = (uint32_t)(int)args[0];
        vc pubkey = args[1];
        vc pseudo = tox_pubkey_to_pseudo_uid(pubkey);
        tox_uid_cache_add(pseudo);
        GRTLOG("tox: typing fn=%d typing=%s", (int)fn, (const char *)args[2]);
        se_emit(SE_TOX_TYPING, pseudo, args[2]);

    } else if(strcmp(type, "friend_status") == 0 && args.num_elems() >= 3) {
        uint32_t fn = (uint32_t)(int)args[0];
        vc pubkey = args[1];
        vc status = args[2];
        vc pseudo = tox_pubkey_to_pseudo_uid(pubkey);
        tox_uid_cache_add(pseudo);
        (void)fn;
        GRTLOG("tox: friend status fn=%d status=%s", (int)fn, (const char *)status);
        se_emit(SE_TOX_FRIEND_USER_STATUS, pseudo, status);
    }
}

static void
on_tox_event(const char *type, const vc &args, void *userdata)
{
    (void)userdata;
    if(!Started)
        return;
    process_tox_event(type, args);
}

int
tox_bridge_init(const char *save_file)
{
    if(Started)
        return 1;

    Tox_plugin = toxp_init(save_file, on_tox_event, NULL);
    if(!Tox_plugin)
        return 0;

    Started = 1;
    if(!Tox_q)
    {
        Tox_q = new ToxQueue;
        if(!Tox_q->init())
        {
            delete Tox_q;
            Tox_q = 0;
        }
    }
    if(Tox_q)
        Tox_q->recover_inprogress();
    tox_bridge_cleanup_incomplete();
    tox_bridge_rebuild_friend_cache();
    safe_add_crdt_tag(to_hex(My_UID), "_tox_device");
    GRTLOG("tox bridge: initialized", 0, 0);
    return 1;
}

void
tox_bridge_shutdown()
{
    if(!Started)
        return;

    // clean up outgoing file transfers
    for(std::map<DwString, OutgoingFileTransfer>::iterator it = Outgoing_xfers.begin();
        it != Outgoing_xfers.end(); ++it)
    {
        if(it->second.fd >= 0)
            close(it->second.fd);
    }
    Outgoing_xfers.clear();

    if(Tox_q) {
        Tox_q->exit();
        delete Tox_q;
        Tox_q = 0;
    }

    if(Tox_plugin) {
        toxp_save(Tox_plugin);
        toxp_shutdown(Tox_plugin);
        Tox_plugin = 0;
    }
    Started = 0;
    GRTLOG("tox bridge: shutdown", 0, 0);
}

void
tox_bridge_cleanup_incomplete()
{
    DwString pat = newfn("*.tmp");
    FindVec fv(pat);
    for(int i = 0; i < fv.num_elems(); ++i)
    {
        const WIN32_FIND_DATA &d = *fv[i];
        DwString full = newfn(d.cFileName);
        DeleteFile(full.c_str());
        GRTLOG("tox: cleaned orphaned temp file %s", d.cFileName, 0);
    }
}

int
tox_bridge_is_active()
{
    return Started;
}

ToxQueue *
tox_queue()
{
    return Tox_q;
}

void
tox_bridge_poll()
{
    if(!Started || !Tox_plugin)
        return;

    toxp_iterate(Tox_plugin);

    // reset stale in-progress text-only messages so they get retried
    // (handles lost read receipts). file transfers are one-shot and must
    // not be retried.
    if(Tox_q)
        Tox_q->sql_simple("UPDATE tox_outbox SET status=0 "
                           "WHERE status=1 AND (has_file IS NULL OR has_file = 0) "
                           "AND created_at < strftime('%s','now') - 30");

    tox_bridge_send_queued();
}

vc
tox_bridge_get_address()
{
    if(!Tox_plugin)
        return vcnil;
    vc result = toxp_get_address(Tox_plugin);
    vc addr;
    if(!result.find("address", addr))
        return vcnil;
    return addr;
}

vc
tox_bridge_get_name()
{
    if(!Tox_plugin)
        return vcnil;
    vc result = toxp_get_name(Tox_plugin);
    vc name;
    if(!result.find("name", name))
        return vcnil;
    return name;
}

vc
tox_bridge_get_status_message()
{
    if(!Tox_plugin)
        return vcnil;
    vc result = toxp_get_status_message(Tox_plugin);
    vc msg;
    if(!result.find("status_message", msg))
        return vcnil;
    return msg;
}

vc
tox_bridge_get_pubkey()
{
    if(!Tox_plugin)
        return vcnil;
    vc result = toxp_get_self_pubkey(Tox_plugin);
    vc pk;
    if(!result.find("pubkey", pk))
        return vcnil;
    return pk;
}

int
tox_bridge_friend_add(const vc &address, const vc &message)
{
    if(!Tox_plugin)
        return 0;
    uint32_t fn;
    if(toxp_friend_add(Tox_plugin, address, message, &fn))
    {
        vc pseudo(address, 10);
        tox_uid_cache_add(pseudo);
        return 1;
    }
    return 0;
}

int
tox_bridge_friend_add_norequest(const vc &pubkey)
{
    if(!Tox_plugin)
        return 0;
    uint32_t fn;
    if(toxp_friend_add_norequest(Tox_plugin, pubkey, &fn))
    {
        vc pseudo = tox_pubkey_to_pseudo_uid(pubkey);
        tox_uid_cache_add(pseudo);
        return 1;
    }
    return 0;
}

int
tox_bridge_friend_delete(uint32_t friend_number)
{
    if(!Tox_plugin)
        return 0;
    return toxp_friend_delete(Tox_plugin, friend_number);
}

int
tox_bridge_friend_delete_by_pubkey(const vc &pubkey)
{
    vc pseudo = tox_pubkey_to_pseudo_uid(pubkey);
    uint32_t fn;
    if(!tox_pseudo_uid_to_friend_number(pseudo, &fn))
        return 0;
    return tox_bridge_friend_delete(fn);
}

int
tox_bridge_send_message(uint32_t friend_number, const vc &text, int is_action, uint32_t *mid_out, int *tox_error_out)
{
    if(!Tox_plugin)
        return -1;
    uint32_t mid = 0;
    int tox_err = 0;
    int ret = toxp_message_send(Tox_plugin, friend_number, text, is_action, &mid, &tox_err);
    if(ret == 0 && tox_err != 0)
    {
        if(tox_error_out)
            *tox_error_out = tox_err;
        return 0;
    }
    if(ret == 1 && mid_out)
        *mid_out = mid;
    return ret;
}

int
tox_bridge_file_send(uint32_t friend_number, const vc &name, uint64_t size)
{
    if(!Tox_plugin)
        return 0;
    uint32_t fnum;
    return toxp_file_send(Tox_plugin, friend_number, name, size, &fnum);
}

int
tox_bridge_file_send_data(uint32_t friend_number, uint32_t file_number,
                          uint64_t pos, const vc &data)
{
    if(!Tox_plugin)
        return 0;
    return toxp_file_send_data(Tox_plugin, friend_number, file_number, pos, data);
}

int
tox_bridge_file_accept(uint32_t friend_number, uint32_t file_number)
{
    if(!Tox_plugin)
        return 0;
    return toxp_file_accept(Tox_plugin, friend_number, file_number, 0);
}

int
tox_pseudo_uid_to_friend_number(const vc &pseudo_uid, uint32_t *fn_out)
{
    if(Friend_cache.is_nil())
        tox_bridge_rebuild_friend_cache();
    if(Friend_cache.is_nil())
        return 0;
    int n = Friend_cache.num_elems();
    for(int i = 0; i < n; ++i)
    {
        vc entry = Friend_cache[i];
        vc pubkey;
        if(entry.find("pubkey", pubkey))
        {
            vc pseudo = tox_pubkey_to_pseudo_uid(pubkey);
            if(pseudo == pseudo_uid)
            {
                vc fnum;
                if(entry.find("friend_number", fnum) && fnum.type() == VC_INT)
                {
                    if(fn_out)
                        *fn_out = (uint32_t)(int)fnum;
                    return 1;
                }
            }
        }
    }
    return 0;
}

int
tox_friend_number_to_pseudo_uid(uint32_t fn, vc &pseudo_uid_out)
{
    if(Friend_cache.is_nil())
        tox_bridge_rebuild_friend_cache();
    if(Friend_cache.is_nil())
        return 0;
    int n = Friend_cache.num_elems();
    for(int i = 0; i < n; ++i)
    {
        vc entry = Friend_cache[i];
        vc fnum;
        if(entry.find("friend_number", fnum) && fnum == vc((int)fn))
        {
            vc pubkey;
            if(entry.find("pubkey", pubkey))
                pseudo_uid_out = tox_pubkey_to_pseudo_uid(pubkey);
            else
                return 0;
            return 1;
        }
    }
    return 0;
}


void
tox_bridge_rebuild_friend_cache()
{
    if(!Tox_plugin)
    {
        Friend_cache = vcnil;
        return;
    }
    vc result = toxp_friend_list(Tox_plugin);
    vc fl;
    if(result.find("friends", fl) && fl.type() == VC_VECTOR)
    {
        Friend_cache = fl;
        int n = Friend_cache.num_elems();
        for(int i = 0; i < n; ++i)
        {
            vc entry = Friend_cache[i];
            vc pubkey;
            if(entry.find("pubkey", pubkey))
                tox_uid_cache_add(tox_pubkey_to_pseudo_uid(pubkey));
        }
        for(int i = 0; i < n; ++i)
        {
            vc entry = Friend_cache[i];
            vc pubkey;
            vc name;
            if(entry.find("pubkey", pubkey) && entry.find("name", name) && !name.is_nil() && name != vc(""))
            {
                vc pseudo = tox_pubkey_to_pseudo_uid(pubkey);
                Session_infos.add_kv(pseudo, make_tox_info_vec(pseudo, name));
                DwString composite;
                composite += to_hex(pseudo);
                composite += "_";
                composite += to_hex(name);
                safe_add_crdt_tag(composite, "_tox_friend");
            }
        }
        GRTLOG("tox bridge: rebuilt friend cache, count=%d", Friend_cache.num_elems(), 0);
    }
}

int
tox_bridge_is_tox_uid(const vc &uid)
{
    if(!Tox_q)
    {
        Tox_q = new ToxQueue;
        if(!Tox_q->init())
        {
            delete Tox_q;
            Tox_q = 0;
        }
    }
    if(Tox_q)
    {
        vc res = Tox_q->sql_simple("SELECT 1 FROM tox_uid_type WHERE pseudo_uid=?1", vcblob(uid));
        if(!res.is_nil() && res.num_elems() > 0)
            return 1;
    }
    if(Friend_cache.is_nil())
        tox_bridge_rebuild_friend_cache();
    if(Friend_cache.is_nil())
        return 0;
    int n = Friend_cache.num_elems();
    for(int i = 0; i < n; ++i)
    {
        vc entry = Friend_cache[i];
        vc pubkey;
        if(entry.find("pubkey", pubkey))
        {
            vc pseudo = tox_pubkey_to_pseudo_uid(pubkey);
            if(pseudo == uid)
                return 1;
        }
    }
    return 0;
}

int
tox_bridge_get_self_public_key(char **out, int *len_out)
{
    vc pk = tox_bridge_get_pubkey();
    if(pk.is_nil())
        return 0;
    if(out)
    {
        *out = new char[pk.len()];
        memcpy(*out, (const char *)pk, pk.len());
    }
    if(len_out)
        *len_out = pk.len();
    return 1;
}

vc
tox_bridge_get_friend_list_vc()
{
    if(Friend_cache.is_nil())
        tox_bridge_rebuild_friend_cache();
    return Friend_cache;
}

int
tox_bridge_set_typing(uint32_t friend_number, int typing)
{
    if(!Tox_plugin)
        return 0;
    return toxp_typing_set(Tox_plugin, friend_number, typing);
}

int
tox_bridge_set_typing_by_uid(const vc &pseudo_uid, int typing)
{
    uint32_t fn;
    if(!tox_pseudo_uid_to_friend_number(pseudo_uid, &fn))
        return 0;
    return tox_bridge_set_typing(fn, typing);
}

int
tox_bridge_set_name(const char *name, int name_len)
{
    if(!Tox_plugin)
        return 0;
    return toxp_set_name(Tox_plugin, name, name_len);
}

int
tox_bridge_set_status_message(const char *msg, int msg_len)
{
    if(!Tox_plugin)
        return 0;
    return toxp_set_status_message(Tox_plugin, msg, msg_len);
}

int
tox_bridge_set_user_status(const char *status)
{
    if(!Tox_plugin)
        return 0;
    toxp_set_user_status(Tox_plugin, status);
    return 1;
}

vc
tox_bridge_get_user_status()
{
    if(!Tox_plugin)
        return vcnil;
    vc result = toxp_get_user_status(Tox_plugin);
    vc status;
    if(!result.find("status", status))
        return vcnil;
    return status;
}

// --- ToxQueue implementation ---

ToxQueue::ToxQueue() : SimpleSql("tox.sql") {}

void
ToxQueue::init_schema(const DwString&)
{
    sql_simple("CREATE TABLE IF NOT EXISTS tox_outbox ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "qqm_blob BLOB, "
               "recipient_pseudo BLOB, "
               "status INTEGER DEFAULT 0, "
               "tox_mid INTEGER, "
               "local_mid TEXT, "
               "has_file INTEGER DEFAULT 0, "
               "created_at INTEGER DEFAULT (strftime('%s','now')))");
    sql_simple("CREATE TABLE IF NOT EXISTS tox_uid_type ("
               "pseudo_uid BLOB PRIMARY KEY)");
    // migration: add has_file column for existing databases
    // PRAGMA table_info is the standard SQLite way to check column existence
    vc col_info = sql_simple("PRAGMA table_info(tox_outbox)");
    int has_col = 0;
    if(!col_info.is_nil())
    {
        int n = col_info.num_elems();
        for(int i = 0; i < n; ++i)
        {
            vc col_name = col_info[i][1];
            if(col_name.type() == VC_STRING)
            {
                DwString s((const char *)col_name, col_name.len());
                if(s == "has_file")
                {
                    has_col = 1;
                    break;
                }
            }
        }
    }
    if(!has_col)
        (void)sql_simple("ALTER TABLE tox_outbox ADD COLUMN has_file INTEGER DEFAULT 0");
}

int
ToxQueue::enqueue(const vc &qqm_blob, const vc &recipient_pseudo, const vc &local_mid, int has_file)
{
    vc blob_arg = vcblob(qqm_blob);
    vc pseudo_arg = vcblob(recipient_pseudo);
    vc res = sql_simple("INSERT INTO tox_outbox (qqm_blob, recipient_pseudo, status, local_mid, has_file) "
                        "VALUES (?1, ?2, 0, ?3, ?4)",
                        blob_arg, pseudo_arg, local_mid, vc(has_file));
    return !res.is_nil();
}

vc
ToxQueue::dequeue(vc *recipient_pseudo_out, vc *local_mid_out, int64_t *row_id)
{
    vc res = sql_simple("SELECT id, recipient_pseudo, local_mid FROM tox_outbox "
                        "WHERE status=0 ORDER BY id LIMIT 1");
    if(res.is_nil() || res.num_elems() == 0)
        return vcnil;
    vc row = res[0];
    if(row_id) *row_id = (int64_t)(long long)row[0];
    if(recipient_pseudo_out) *recipient_pseudo_out = row[1];
    if(local_mid_out) *local_mid_out = row[2];
    return vctrue;
}

int
ToxQueue::mark_inprogress(int64_t row_id, uint32_t tox_mid)
{
    vc res = sql_simple("UPDATE tox_outbox SET status=1, tox_mid=?2, "
                        "created_at=strftime('%s','now') "
                        "WHERE id=?1",
                        vc((long long)row_id), vc((int)tox_mid));
    return !res.is_nil();
}

int
ToxQueue::mark_sent(int64_t row_id)
{
    vc res = sql_simple("DELETE FROM tox_outbox WHERE id=?1",
                        vc((long long)row_id));
    return !res.is_nil();
}

int
ToxQueue::mark_failed(int64_t row_id)
{
    vc res = sql_simple("UPDATE tox_outbox SET status=3 WHERE id=?1",
                        vc((long long)row_id));
    return !res.is_nil();
}

int
ToxQueue::remove_message(const vc &local_mid)
{
    vc res = sql_simple("DELETE FROM tox_outbox WHERE local_mid=?1",
                        local_mid);
    return !res.is_nil();
}

vc
ToxQueue::load_qqm_blob(int64_t row_id)
{
    vc res = sql_simple("SELECT qqm_blob FROM tox_outbox WHERE id=?1",
                        vc((long long)row_id));
    if(res.is_nil() || res.num_elems() == 0)
        return vcnil;
    return res[0][0];
}

void
ToxQueue::recover_inprogress()
{
    // text-only: can retry on restart
    sql_simple("UPDATE tox_outbox SET status=0 WHERE status=1 "
               "AND (has_file IS NULL OR has_file = 0)");
    // file messages: one-shot, mark as failed on restart
    sql_simple("UPDATE tox_outbox SET status=3 WHERE status=1 AND has_file = 1");
}

vc
ToxQueue::get_qd_msgs(const vc &pseudo_uid)
{
    vc ret(VC_VECTOR);
    vc rows;
    if(pseudo_uid.is_nil())
        rows = sql_simple("SELECT local_mid, recipient_pseudo, qqm_blob "
                          "FROM tox_outbox WHERE status IN (0,1,3) ORDER BY id");
    else
        rows = sql_simple("SELECT local_mid, recipient_pseudo, qqm_blob "
                          "FROM tox_outbox WHERE recipient_pseudo=?1 "
                          "AND status IN (0,1,3) ORDER BY id",
                          vcblob(pseudo_uid));
    if(rows.is_nil())
        return ret;
    int n = rows.num_elems();
    for(int i = 0; i < n; ++i)
    {
        vc row = rows[i];
        vc local_mid = row[0];
        vc recipient = row[1];
        vc blob = row[2];
        vc qqm;
        deserialize(blob, qqm);
        vc v(VC_VECTOR);
        v[0] = recipient;
        v[1] = local_mid;
        v[2] = qqm[QQM_MSG_VEC][QQM_BODY_LOGICAL_CLOCK];
        v[3] = qqm[QQM_MSG_VEC][QQM_BODY_ATTACHMENT];
        v[4] = qqm[QQM_MSG_VEC][QQM_BODY_SPECIAL_TYPE];
        ret.append(v);
    }
    return ret;
}

vc
ToxQueue::load_qd_body(const vc &local_mid)
{
    vc res = sql_simple("SELECT qqm_blob FROM tox_outbox WHERE local_mid=?1",
                        local_mid);
    if(res.is_nil() || res.num_elems() == 0)
        return vcnil;
    vc blob = res[0][0];
    vc qqm;
    deserialize(blob, qqm);
    return direct_to_body2(qqm[QQM_MSG_VEC]);
}

int
tox_bridge_send_message_by_uid(const vc &pseudo_uid, const vc &text, int is_action)
{
    if(!Tox_q)
        return 0;
    uint32_t fn;
    if(!tox_pseudo_uid_to_friend_number(pseudo_uid, &fn))
        return 0;
    vc local_mid = to_hex(gen_id());
    vc body = make_msg_body(My_UID, text);
    vc qqm(VC_VECTOR);
    qqm[QQM_RECIP_VEC] = vc(VC_VECTOR);
    qqm[QQM_RECIP_VEC][0] = pseudo_uid;
    qqm[QQM_MSG_VEC] = body;
    qqm[QQM_LOCAL_ID] = local_mid;
    vc blob = serialize(qqm);
    return Tox_q->enqueue(blob, pseudo_uid, local_mid);
}

int
tox_bridge_send_file_message_by_uid(const vc &pseudo_uid, const vc &text,
                                     const vc &original_filename,
                                     const DwString &attachment_basename,
                                     const vc &filehash,
                                     uint64_t file_size,
                                     vc &local_mid_out)
{
    if(!Tox_q)
        return 0;
    uint32_t fn;
    if(!tox_pseudo_uid_to_friend_number(pseudo_uid, &fn))
        return 0;
    vc local_mid = to_hex(gen_id());
    local_mid_out = local_mid;
    vc body = make_msg_body(My_UID, text,
                            attachment_basename,
                            original_filename,
                            vc((long long)file_size));
    (void)filehash;
    vc qqm(VC_VECTOR);
    qqm[QQM_RECIP_VEC] = vc(VC_VECTOR);
    qqm[QQM_RECIP_VEC][0] = pseudo_uid;
    qqm[QQM_MSG_VEC] = body;
    qqm[QQM_LOCAL_ID] = local_mid;
    vc blob = serialize(qqm);
    return Tox_q->enqueue(blob, pseudo_uid, local_mid, 1);
}

void
tox_bridge_send_queued()
{
    if(!Tox_q)
        return;
    vc recipient_pseudo;
    vc local_mid;
    int64_t row_id = -1;
    if(Tox_q->dequeue(&recipient_pseudo, &local_mid, &row_id).is_nil())
        return;
    uint32_t fn;
    if(!tox_pseudo_uid_to_friend_number(recipient_pseudo, &fn))
    {
        Tox_q->mark_failed(row_id);
        se_emit_msg(SE_MSG_SEND_FAIL, local_mid, recipient_pseudo);
        return;
    }
    vc qqm_blob = Tox_q->load_qqm_blob(row_id);
    if(qqm_blob.is_nil())
    {
        Tox_q->mark_failed(row_id);
        se_emit_msg(SE_MSG_SEND_FAIL, local_mid, recipient_pseudo);
        return;
    }
    vc qqm;
    deserialize(qqm_blob, qqm);
    vc body = qqm[QQM_MSG_VEC];
    vc text = body[QQM_BODY_NEW_TEXT];

    // if this message has a file attachment, skip text send entirely
    // (one-shot: if it fails, the message is marked failed permanently)
    if(!body[QQM_BODY_ATTACHMENT].is_nil())
    {
        DwString basename((const char *)body[QQM_BODY_ATTACHMENT],
                          body[QQM_BODY_ATTACHMENT].len());
        DwString file_path = newfn(basename);
        vc orig_name_vc = body[QQM_BODY_FILE_ATTACHMENT];
        DwString orig_name;
        if(!orig_name_vc.is_nil())
            orig_name = DwString((const char *)orig_name_vc, orig_name_vc.len());
        else
            orig_name = DwString("file");
        uint64_t fsize = 0;
        if(body[QQM_BODY_ESTIMATED_SIZE].type() == VC_INT)
            fsize = (uint64_t)(long long)body[QQM_BODY_ESTIMATED_SIZE];

        struct stat st;
        if(stat(file_path.c_str(), &st) != 0 || (uint64_t)st.st_size != fsize)
        {
            Tox_q->mark_failed(row_id);
            se_emit_msg(SE_MSG_SEND_FAIL, local_mid, recipient_pseudo);
            GRTLOG("tox: file missing for message, marking failed %s", file_path.c_str(), 0);
            return;
        }

        uint32_t fnum;
        vc name_vc(VC_BSTRING, orig_name.c_str(), (long)orig_name.length());
        if(!toxp_file_send(Tox_plugin, fn, name_vc, fsize, &fnum))
        {
            Tox_q->mark_failed(row_id);
            se_emit_msg(SE_MSG_SEND_FAIL, local_mid, recipient_pseudo);
            GRTLOG("tox: file send init failed for fn=%d", (int)fn, 0);
            return;
        }

        // no text sent, so tox_mid is 0 (no read receipt expected)
        Tox_q->mark_inprogress(row_id, 0);

        OutgoingFileTransfer xfer;
        xfer.file_path = file_path;
        xfer.fd = -1;
        xfer.file_size = fsize;
        xfer.sent_size = 0;
        xfer.original_filename = orig_name;
        xfer.row_id = row_id;
        xfer.local_mid = local_mid;
        xfer.tox_fn = fn;
        xfer.tox_fnum = fnum;
        Outgoing_xfers[xfer_key(fn, fnum)] = xfer;
        GRTLOG("tox: file send initiated fn=%d fnum=%d", (int)fn, (int)fnum);
        return;
    }

    // text-only path
    uint32_t tox_mid = 0;
    int tox_error = -1;
    int ret = tox_bridge_send_message(fn, text, 0, &tox_mid, &tox_error);
    if(ret == 1 || (ret == 0 && tox_error == TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_CONNECTED))
    {
        // toxcore accepted the message (friend offline, will deliver later)
        Tox_q->mark_inprogress(row_id, tox_mid);
    }
    else if(ret == 0)
    {
        // permanent failure
        Tox_q->mark_failed(row_id);
        se_emit_msg(SE_MSG_SEND_FAIL, local_mid, recipient_pseudo);
        GRTLOG("tox: send permanent failure, error=%d", tox_error, 0);
    }
    else
    {
        // transport failure: mark in-progress so we don't re-dequeue
        // on the next poll and spam the recipient.  The stale-timeout in
        // tox_bridge_poll (30 s) will reset stuck entries to status=0
        // for retry.
        Tox_q->mark_inprogress(row_id, tox_mid);
    }
}

int
tox_bridge_kill_message(const vc &local_mid)
{
    if(!Tox_q)
        return 0;
    DwString s = local_mid;
    if(fn_extension(s) == ".q")
        return 0;
    vc res = Tox_q->sql_simple("SELECT recipient_pseudo FROM tox_outbox WHERE local_mid=?1",
                                local_mid);
    if(res.is_nil() || res.num_elems() == 0)
        return 0;
    vc recipient = res[0][0];
    Tox_q->remove_message(local_mid);
    se_emit_msg(SE_MSG_SEND_CANCELED, local_mid, recipient);
    return 1;
}

}

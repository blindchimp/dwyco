#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#include "toxbridge.h"
#include "qauth.h"
#include "vcxstrm.h"
#include "vccomp.h"
#include "dwrtlog.h"
#include "se.h"
#include "qmsg.h"
#include "ser.h"
#include "simplesql.h"
#include "xinfo.h"
#include "fnmod.h"

#define RPCM_METHOD 0
#define RPCM_PARAMS 1
#define RPCM_REQID 2

#define RPCM_RES_REQID 0
#define RPCM_RESULT 1

#define EV_TYPE 0
#define EV_ARGS 1

// toxcore error codes — only the ones we check on the bridge side
#define TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_CONNECTED 3

namespace dwyco {

static int Toxd_pid;
static int Toxd_stdin = -1;
static int Toxd_stdout = -1;
static int Reqid_counter;
static int Started;
static ToxQueue *Tox_q;
static vc Friend_cache;
static DwString Toxd_path;
static DwString Data_dir;
static time_t Toxd_last_restart;
static int Toxd_restart_delay = 1;
static const int Toxd_max_restart_delay = 300;
static int Inhibit_restart;

// forward declarations
static int toxd_rpc_call(const vc &method, const vc &params, vc &result_out, int timeout_ms);
static int toxd_start();
static void toxd_stop();

struct toxd_error {
    DwString error;
    toxd_error(const DwString& e) : error(e) {}
};



static int
read_all(int fd, char *buf, int len)
{
    int total = 0;
    while(total < len) {
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;
        int ret;
        do {
            ret = poll(&pfd, 1, 10000);
        } while(ret < 0 && errno == EINTR);
        if(ret < 0)
            throw toxd_error(DwString(strerror(errno)));
        if(ret == 0)
            throw toxd_error(DwString("read timeout"));

        int n = (int)read(fd, buf + total, (size_t)(len - total));
        if(n <= 0) {
            if(n == 0)
                throw toxd_error(DwString("toxd read eof"));
            if(errno == EINTR)
                continue;
            throw toxd_error(DwString(strerror(errno)));
        }
        total += n;
    }
    return total;
}

static int
write_all(int fd, const char *buf, int len)
{
    int total = 0;
    while(total < len) {
        int n = (int)write(fd, buf + total, (size_t)(len - total));
        if(n <= 0) {
            if(errno == EINTR)
                continue;
            throw toxd_error(DwString(strerror(errno)));
        }
        total += n;
    }
    return total;
}

static int
write_msg(int fd, vc msg)
{
    vcxstream os(0, 2048, vcxstream::CONTINUOUS);
    if(!os.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
        throw toxd_error(DwString("write_msg open failed"));
    vc_composite::new_dfs();
    if(msg.xfer_out(os) < 0) {
        os.close(vcxstream::DISCARD);
        throw toxd_error(DwString("write_msg xfer failed"));
    }
    const char *buf;
    long len;
    os.cur_buf(buf, len);
    uint32_t nlen = htonl((uint32_t)len);
    write_all(fd, (const char *)&nlen, 4);
    write_all(fd, buf, len);
    return 1;
}

static vc
read_msg(int fd)
{
    uint32_t nlen;
    read_all(fd, (char *)&nlen, 4);
    long len = (long)ntohl(nlen);
    if(len < 2 || len > 1024)
        throw toxd_error(DwString("read_msg invalid length"));
    char *buf = new char[len];
    try {
        read_all(fd, buf, (int)len);
    } catch(...) {
        delete[] buf;
        throw;
    }
    vcxstream is(buf, len, vcxstream::FIXED);
    if(!is.open(vcxstream::READABLE)) {
        delete[] buf;
        throw toxd_error(DwString("read_msg stream open failed"));
    }
    vc msg;
    if(msg.xfer_in(is) < 0) {
        delete[] buf;
        throw toxd_error(DwString("read_msg xfer failed"));
    }
    delete[] buf;
    return msg;
}

static int
is_response(const vc &msg)
{
    if(msg.type() != VC_VECTOR)
        return 0;
    if(msg.num_elems() < 2)
        return 0;
    return msg[0].type() == VC_INT;
}

static int
is_event(const vc &msg)
{
    if(msg.type() != VC_VECTOR)
        return 0;
    if(msg.num_elems() < 1)
        return 0;
    return msg[0].type() == VC_STRING;
}

vc
tox_pubkey_to_pseudo_uid(const vc &pubkey)
{
    if(pubkey.len() < 10)
        return vcnil;
    return vc(VC_BSTRING, (const char *)pubkey, 10);
}

static vc
vcblob(const vc &v)
{
    vc ret(VC_VECTOR);
    ret.append(vc("blob"));
    ret.append(v);
    return ret;
}

static void
tox_uid_cache_add(const vc &pseudo_uid)
{
    if(Tox_q)
        Tox_q->sql_simple("INSERT OR IGNORE INTO tox_uid_type VALUES(?1)", vcblob(pseudo_uid));
}

static void
toxd_stop()
{
    if(Toxd_pid <= 0 && Toxd_stdin < 0 && Toxd_stdout < 0)
        return;

    if(Toxd_stdin >= 0) {
        close(Toxd_stdin);
        Toxd_stdin = -1;
    }

    if(Toxd_pid > 0) {
        int status;
        if(waitpid(Toxd_pid, &status, WNOHANG) == 0) {
            kill(Toxd_pid, SIGKILL);
            waitpid(Toxd_pid, NULL, 0);
        }
        Toxd_pid = 0;
    }

    if(Toxd_stdout >= 0) {
        close(Toxd_stdout);
        Toxd_stdout = -1;
    }

    Started = 0;
}

static int
toxd_start()
{
    if(Started || Toxd_pid > 0)
        return 1;

    if(access(Toxd_path.c_str(), X_OK) != 0) {
        GRTLOG("tox bridge: toxd not found at %s", Toxd_path.c_str(), 0);
        return 0;
    }

    int stdin_pipe[2];
    int stdout_pipe[2];

    if(pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0)
        return 0;

    Toxd_pid = fork();
    if(Toxd_pid < 0) {
        close(stdin_pipe[0]); close(stdin_pipe[1]);
        close(stdout_pipe[0]); close(stdout_pipe[1]);
        return 0;
    }

    if(Toxd_pid == 0) {
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);

        if(Data_dir.length() > 0)
            setenv("DWYCO_TOX_DATA", Data_dir.c_str(), 1);

        execl(Toxd_path.c_str(), Toxd_path.c_str(), (char *)NULL);
        _exit(1);
    }

    close(stdin_pipe[0]);
    close(stdout_pipe[1]);
    Toxd_stdin = stdin_pipe[1];
    Toxd_stdout = stdout_pipe[0];

    int ready = 0;
    try {
        int timeout = 10000;
        int elapsed = 0;
        while(elapsed < timeout) {
            struct pollfd pfd;
            pfd.fd = Toxd_stdout;
            pfd.events = POLLIN;
            int ret;
            do {
                ret = poll(&pfd, 1, 50);
            } while(ret < 0 && errno == EINTR);
            if(ret < 0)
                throw toxd_error(DwString(strerror(errno)));
            if(ret == 0) {
                elapsed += 50;
                continue;
            }
            vc msg = read_msg(Toxd_stdout);
            if(is_event(msg) && strcmp((const char *)msg[EV_TYPE], "ready") == 0) {
                ready = 1;
                break;
            }
        }
    } catch(toxd_error&) {
    }

    if(!ready) {
        toxd_stop();
        return 0;
    }

    Started = 1;
    Reqid_counter = 0;
    GRTLOG("tox bridge: started, pid=%d", Toxd_pid, 0);
    return 1;
}

static void
process_tox_event(const vc &ev)
{
    vc type_vc = ev[EV_TYPE];
    vc args = ev[EV_ARGS];
    const char *type = (const char *)type_vc;

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

        vc body(VC_VECTOR);
        body[QQM_BODY_FROM] = pseudo;
        body[QQM_BODY_TEXT_do_not_use] = vcnil;
        body[QQM_BODY_ATTACHMENT] = vcnil;
        body[QQM_BODY_DATE] = date_vector();
        body[QQM_BODY_AUTH_VEC] = vcnil;
        body[QQM_BODY_FORWARDED_BODY] = vcnil;
        body[QQM_BODY_NEW_TEXT] = text;
        body[QQM_BODY_ATTACHMENT_LOCATION] = vcnil;
        body[QQM_BODY_SPECIAL_TYPE] = vcnil;
        body[QQM_BODY_NO_FORWARD] = vcnil;
        body[QQM_BODY_FILE_ATTACHMENT] = vcnil;
        body[QQM_BODY_DHSF] = vcnil;
        body[QQM_BODY_EMSG] = vcnil;
        body[QQM_BODY_ESTIMATED_SIZE] = vcnil;
        body[QQM_BODY_NO_DELIVERY_REPORT] = vcnil;
        body[QQM_BODY_LOGICAL_CLOCK] = (int64_t)++Logical_clock;
        body[QQM_BODY_FROM_GROUP] = vcnil;

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

    } else if(strcmp(type, "file_request") == 0 && args.num_elems() >= 5) {
        uint32_t fn = (uint32_t)(int)args[0];
        uint32_t fnum = (uint32_t)(int)args[1];
        uint32_t kind = (uint32_t)(int)args[2];
        (void)kind;
        uint64_t size = 0;
        if(args[3].type() == VC_INT)
            size = (uint64_t)(long long)args[3];
        vc name = args[4];
        vc pseudo;
        if(tox_friend_number_to_pseudo_uid(fn, pseudo))
        {
            se_emit(SE_TOX_FILE_REQUEST, pseudo, vc((int)fnum), name, vc((long long)size));
        }

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
        se_emit(SE_TOX_FILE_CHUNK, vc((int)fn), vc((int)fnum),
                vc((long long)pos), data);

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

static int
read_pending_events(int fd)
{
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    int count = 0;

    try {
        int ret;
        while(true) {
            do {
                ret = poll(&pfd, 1, 0);
            } while(ret < 0 && errno == EINTR);
            if(ret < 0)
                throw toxd_error(DwString(strerror(errno)));
            if(ret == 0 || !(pfd.revents & POLLIN))
                break;
            vc msg = read_msg(fd);
            if(is_event(msg)) {
                process_tox_event(msg);
                ++count;
            }
        }
    } catch(toxd_error&) {
        se_emit(SE_TOX_CRASHED, vcnil);
        toxd_stop();
    }
    return count;
}

static int
read_response(int fd, int reqid, vc &result_out, int timeout_ms)
{
    int elapsed = 0;
    while(elapsed < timeout_ms) {
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;
        int ret;
        do {
            ret = poll(&pfd, 1, 50);
        } while(ret < 0 && errno == EINTR);
        if(ret < 0)
            throw toxd_error(DwString(strerror(errno)));
        if(ret == 0) {
            elapsed += 50;
            continue;
        }
        vc msg = read_msg(fd);
        if(is_event(msg)) {
            process_tox_event(msg);
            continue;
        }
        if(is_response(msg) && (int)msg[RPCM_RES_REQID] == reqid) {
            result_out = msg[RPCM_RESULT];
            return 1;
        }
    }
    return 0;
}

static int
toxd_rpc_call(const vc &method, const vc &params, vc &result_out, int timeout_ms)
{
    try {
        if(!Started)
            return 0;

        int reqid = ++Reqid_counter;
        vc req(VC_VECTOR);
        req.append(method);
        req.append(params);
        req.append(vc(reqid));

        write_msg(Toxd_stdin, req);

        if(!read_response(Toxd_stdout, reqid, result_out, timeout_ms))
            return 0;

        if(result_out.type() == VC_STRING)
            return 0;

        return 1;
    } catch(toxd_error&) {
        se_emit(SE_TOX_CRASHED, vcnil);
        toxd_stop();
        return 0;
    }
}

int
tox_bridge_init(const char *toxd_path, const char *data_dir)
{
    if(Started)
        return 1;

    Toxd_path = toxd_path;
    if(data_dir)
        Data_dir = data_dir;
    else
        Data_dir = DwString();

    if(!toxd_start())
        return 0;

    Inhibit_restart = 0;
    Toxd_last_restart = 0;
    Toxd_restart_delay = 1;
    Tox_q = new ToxQueue;
    if(Tox_q->init())
        Tox_q->recover_inprogress();
    GRTLOG("tox bridge: initialized", 0, 0);
    return 1;
}

void
tox_bridge_shutdown()
{
    if(!Started)
        return;

    if(Tox_q) {
        Tox_q->exit();
        delete Tox_q;
        Tox_q = 0;
    }

    Inhibit_restart = 1;
    toxd_stop();
    GRTLOG("tox bridge: shutdown", 0, 0);
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
    if(!Started && !Inhibit_restart) {
        if(Toxd_last_restart == 0 || time(0) - Toxd_last_restart >= Toxd_restart_delay) {
            Toxd_last_restart = time(0);
            if(toxd_start()) {
                tox_bridge_rebuild_friend_cache();
                if(Tox_q)
                    Tox_q->recover_inprogress();
                Toxd_restart_delay = 1;
                GRTLOG("tox bridge: restarted successfully", 0, 0);
                se_emit(SE_TOX_READY, vcnil);
            } else {
                Toxd_restart_delay *= 2;
                if(Toxd_restart_delay > Toxd_max_restart_delay)
                    Toxd_restart_delay = Toxd_max_restart_delay;
                GRTLOG("tox bridge: restart failed, retry in %ds", Toxd_restart_delay, 0);
            }
        }
        return;
    }

    read_pending_events(Toxd_stdout);

    // reset stale in-progress messages so they get retried
    // (handles lost read receipts and toxd restarts)
    if(Tox_q)
        Tox_q->sql_simple("UPDATE tox_outbox SET status=0 "
                           "WHERE status=1 AND created_at < strftime('%s','now') - 30");

    tox_bridge_send_queued();
}

vc
tox_bridge_get_address()
{
    vc result;
    if(!toxd_rpc_call(vc("get_address"), vc(VC_MAP, "", 0), result, 5000))
        return vcnil;
    vc addr;
    if(!result.find("address", addr))
        return vcnil;
    return addr;
}

vc
tox_bridge_get_name()
{
    vc result;
    if(!toxd_rpc_call(vc("get_name"), vc(VC_MAP, "", 0), result, 5000))
        return vcnil;
    vc name;
    if(!result.find("name", name))
        return vcnil;
    return name;
}

vc
tox_bridge_get_status_message()
{
    vc result;
    if(!toxd_rpc_call(vc("get_status_message"), vc(VC_MAP, "", 0), result, 5000))
        return vcnil;
    vc msg;
    if(!result.find("status_message", msg))
        return vcnil;
    return msg;
}

vc
tox_bridge_get_pubkey()
{
    vc result;
    if(!toxd_rpc_call(vc("generate_keypair"), vc(VC_MAP, "", 0), result, 5000))
        return vcnil;
    vc pk;
    if(!result.find("pubkey", pk))
        return vcnil;
    return pk;
}

int
tox_bridge_friend_add(const vc &address, const vc &message)
{
    vc params(VC_MAP, "", 4);
    params.add_kv("address", address);
    params.add_kv("message", message);
    vc result;
    if(toxd_rpc_call(vc("friend_add"), params, result, 10000))
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
    vc params(VC_MAP, "", 2);
    params.add_kv("pubkey", pubkey);
    vc result;
    if(toxd_rpc_call(vc("friend_add_norequest"), params, result, 10000))
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
    vc params(VC_MAP, "", 2);
    params.add_kv("friend_number", vc((int)friend_number));
    vc result;
    return toxd_rpc_call(vc("friend_delete"), params, result, 5000);
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
    vc params(VC_MAP, "", 6);
    params.add_kv("friend_number", vc((int)friend_number));
    params.add_kv("text", text);
    params.add_kv("type", vc(is_action ? "action" : "normal"));
    vc result;
    if(!toxd_rpc_call(vc("message_send"), params, result, 10000))
        return -1;
    if(tox_error_out)
    {
        vc ec;
        if(result.find("tox_error", ec))
        {
            *tox_error_out = (int)ec;
            return 0;
        }
    }
    if(mid_out)
    {
        vc mid_vc;
        if(result.find("message_id", mid_vc))
            *mid_out = (uint32_t)(int)mid_vc;
    }
    return 1;
}

int
tox_bridge_file_send(uint32_t friend_number, const vc &name, uint64_t size)
{
    vc params(VC_MAP, "", 6);
    params.add_kv("friend_number", vc((int)friend_number));
    params.add_kv("name", name);
    params.add_kv("size", vc((long long)size));
    vc result;
    return toxd_rpc_call(vc("file_send"), params, result, 5000);
}

int
tox_bridge_file_send_data(uint32_t friend_number, uint32_t file_number,
                          uint64_t pos, const vc &data)
{
    vc params(VC_MAP, "", 8);
    params.add_kv("friend_number", vc((int)friend_number));
    params.add_kv("file_number", vc((int)file_number));
    params.add_kv("pos", vc((long long)pos));
    params.add_kv("data", data);
    vc result;
    return toxd_rpc_call(vc("file_send_data"), params, result, 5000);
}

int
tox_bridge_file_accept(uint32_t friend_number, uint32_t file_number)
{
    vc params(VC_MAP, "", 4);
    params.add_kv("friend_number", vc((int)friend_number));
    params.add_kv("file_number", vc((int)file_number));
    vc result;
    return toxd_rpc_call(vc("file_accept"), params, result, 5000);
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
    vc result;
    if(!toxd_rpc_call(vc("friend_list"), vc(VC_MAP, "", 0), result, 5000))
    {
        Friend_cache = vcnil;
        return;
    }
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
        GRTLOG("tox bridge: rebuilt friend cache, count=%d", Friend_cache.num_elems(), 0);
    }
}

int
tox_bridge_is_tox_uid(const vc &uid)
{
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
    vc params(VC_MAP, "", 4);
    params.add_kv("friend_number", vc((int)friend_number));
    params.add_kv("typing", vc(typing ? 1 : 0));
    vc result;
    return toxd_rpc_call(vc("typing_set"), params, result, 5000);
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
    vc params(VC_MAP, "", 4);
    params.add_kv("name", vc(VC_BSTRING, name, (long)name_len));
    vc result;
    return toxd_rpc_call(vc("set_name"), params, result, 5000);
}

int
tox_bridge_set_status_message(const char *msg, int msg_len)
{
    vc params(VC_MAP, "", 4);
    params.add_kv("status_message", vc(VC_BSTRING, msg, (long)msg_len));
    vc result;
    return toxd_rpc_call(vc("set_status_message"), params, result, 5000);
}

int
tox_bridge_set_user_status(const char *status)
{
    vc params(VC_MAP, "", 2);
    params.add_kv("status", vc(status));
    vc result;
    return toxd_rpc_call(vc("set_user_status"), params, result, 5000);
}

vc
tox_bridge_get_user_status()
{
    vc result;
    if(!toxd_rpc_call(vc("get_user_status"), vc(VC_MAP, "", 0), result, 5000))
        return vcnil;
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
               "created_at INTEGER DEFAULT (strftime('%s','now')))");
    sql_simple("CREATE TABLE IF NOT EXISTS tox_uid_type ("
               "pseudo_uid BLOB PRIMARY KEY)");
}

int
ToxQueue::enqueue(const vc &qqm_blob, const vc &recipient_pseudo, const vc &local_mid)
{
    vc blob_arg = vcblob(qqm_blob);
    vc pseudo_arg = vcblob(recipient_pseudo);
    vc res = sql_simple("INSERT INTO tox_outbox (qqm_blob, recipient_pseudo, status, local_mid) "
                        "VALUES (?1, ?2, 0, ?3)",
                        blob_arg, pseudo_arg, local_mid);
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
    sql_simple("UPDATE tox_outbox SET status=0 WHERE status=1");
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
    vc body(VC_VECTOR);
    body[QQM_BODY_FROM] = My_UID;
    body[QQM_BODY_TEXT_do_not_use] = vcnil;
    body[QQM_BODY_ATTACHMENT] = vcnil;
    body[QQM_BODY_DATE] = date_vector();
    body[QQM_BODY_RATING_do_not_use] = vcnil;
    body[QQM_BODY_AUTH_VEC] = vcnil;
    body[QQM_BODY_FORWARDED_BODY] = vcnil;
    body[QQM_BODY_NEW_TEXT] = text;
    body[QQM_BODY_ATTACHMENT_LOCATION] = vcnil;
    body[QQM_BODY_SPECIAL_TYPE] = vcnil;
    body[QQM_BODY_NO_FORWARD] = vcnil;
    body[QQM_BODY_FILE_ATTACHMENT] = vcnil;
    body[QQM_BODY_DHSF] = vcnil;
    body[QQM_BODY_EMSG] = vcnil;
    body[QQM_BODY_ESTIMATED_SIZE] = vcnil;
    body[QQM_BODY_NO_DELIVERY_REPORT] = vcnil;
    body[QQM_BODY_LOGICAL_CLOCK] = (int64_t)++Logical_clock;
    body[QQM_BODY_FROM_GROUP] = vcnil;
    vc qqm(VC_VECTOR);
    qqm[QQM_RECIP_VEC] = vc(VC_VECTOR);
    qqm[QQM_RECIP_VEC][0] = pseudo_uid;
    qqm[QQM_MSG_VEC] = body;
    qqm[QQM_LOCAL_ID] = local_mid;
    vc blob = serialize(qqm);
    return Tox_q->enqueue(blob, pseudo_uid, local_mid);
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
        // RPC transport failure: mark in-progress so we don't re-dequeue
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

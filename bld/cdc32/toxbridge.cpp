#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>

#include "toxbridge.h"
#include "qauth.h"
#include "vcxstrm.h"
#include "vccomp.h"
#include "dwrtlog.h"
#include "se.h"

#define RPCM_METHOD 0
#define RPCM_PARAMS 1
#define RPCM_REQID 2

#define RPCM_RES_REQID 0
#define RPCM_RESULT 1

#define EV_TYPE 0
#define EV_ARGS 1

namespace dwyco {

static int Toxd_pid;
static int Toxd_stdin = -1;
static int Toxd_stdout = -1;
static int Reqid_counter;
static vc Pending_responses;
static int Active;
static pid_t Toxd_pgid;

// forward declaration
static int toxd_rpc_call(const vc &method, const vc &params, vc &result_out, int timeout_ms);

struct toxd_error {};

static void
handle_toxd_crash()
{
    GRTLOG("tox bridge: toxd crashed", 0, 0);
    se_emit(SE_TOX_CRASHED, vcnil);
    if(Toxd_stdin >= 0) {
        close(Toxd_stdin);
        Toxd_stdin = -1;
    }
    if(Toxd_stdout >= 0) {
        close(Toxd_stdout);
        Toxd_stdout = -1;
    }
    if(Toxd_pid > 0) {
        kill(Toxd_pid, SIGKILL);
        int status;
        waitpid(Toxd_pid, &status, WNOHANG);
        Toxd_pid = 0;
    }
    Active = 0;
    Pending_responses = vcnil;
}

static int
read_all(int fd, char *buf, int len)
{
    int total = 0;
    while(total < len) {
        int n = (int)read(fd, buf + total, (size_t)(len - total));
        if(n <= 0) {
            if(n == 0)
                throw toxd_error();
            if(errno == EINTR)
                continue;
            throw toxd_error();
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
            throw toxd_error();
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
        throw toxd_error();
    vc_composite::new_dfs();
    if(msg.xfer_out(os) < 0) {
        os.close(vcxstream::DISCARD);
        throw toxd_error();
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
    if(len <= 0 || len > 1024 * 1024)
        throw toxd_error();
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
        throw toxd_error();
    }
    vc msg;
    if(msg.xfer_in(is) < 0) {
        delete[] buf;
        throw toxd_error();
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
        GRTLOG("tox: friend request from ", 0, 0);
        GRTLOGVC(pseudo);
        se_emit(SE_TOX_FRIEND_REQUEST, pseudo, msg);

    } else if(strcmp(type, "message") == 0 && args.num_elems() >= 4) {
        uint32_t fn = (uint32_t)(int)args[0];
        vc pubkey = args[1];
        vc text = args[2];
        vc msg_type = args[3];
        vc pseudo = tox_pubkey_to_pseudo_uid(pubkey);

        GRTLOG("tox: message from fn ", (int)fn, 0);
        GRTLOGVC(pseudo);

        vc mid(VC_BSTRING, "tox:", 4);
        vc hex_pseudo = to_hex(pseudo);
        mid.append(hex_pseudo);
        mid.append(":");
        vc seq(VC_BSTRING, (const char *)pseudo, 4);
        mid.append(to_hex(seq));

        vc body(VC_VECTOR);
        body.append(mid);
        body.append(pseudo);
        body.append(vcnil);
        body.append(vc((long long)time(0)));
        body.append(vc(0));
        body.append(vcnil);
        body.append(vcnil);
        body.append(vcnil);
        body.append(text);
        body.append(vcnil);

        se_emit(SE_TOX_MESSAGE, pseudo, body, msg_type);

    } else if(strcmp(type, "read_receipt") == 0 && args.num_elems() >= 2) {
        uint32_t fn = (uint32_t)(int)args[0];
        uint32_t mid = (uint32_t)(int)args[1];
        (void)fn;
        GRTLOG("tox: read receipt for mid ", (int)mid, 0);
        se_emit(SE_TOX_READ_RECEIPT, vc((int)mid));

    } else if(strcmp(type, "friend_connection_status") == 0 && args.num_elems() >= 3) {
        uint32_t fn = (uint32_t)(int)args[0];
        vc pubkey = args[1];
        vc status = args[2];
        vc pseudo = tox_pubkey_to_pseudo_uid(pubkey);
        GRTLOG("tox: friend ", (int)fn, status == vc("online") ? 1 : 0);
        se_emit(SE_TOX_FRIEND_STATUS, pseudo, status);

    } else if(strcmp(type, "friend_name") == 0 && args.num_elems() >= 3) {
        uint32_t fn = (uint32_t)(int)args[0];
        vc pubkey = args[1];
        vc name = args[2];
        vc pseudo = tox_pubkey_to_pseudo_uid(pubkey);
        (void)fn;
        se_emit(SE_TOX_FRIEND_NAME, pseudo, name);

    } else if(strcmp(type, "file_request") == 0 && args.num_elems() >= 5) {
        uint32_t fn = (uint32_t)(int)args[0];
        uint32_t fnum = (uint32_t)(int)args[1];
        uint32_t kind = (uint32_t)(int)args[2];
        uint64_t size = 0;
        if(args[3].type() == VC_INT)
            size = (uint64_t)(long long)args[3];
        vc name = args[4];
        (void)kind;
        se_emit(SE_TOX_FILE_REQUEST, vc((int)fn), vc((int)fnum), name, vc((long long)size));

    } else if(strcmp(type, "self_connection_status") == 0 && args.num_elems() >= 1) {
        vc status = args[0];
        GRTLOG("tox: self connection status ", 0, 0);
        GRTLOG((const char *)status, status.len(), 0);
        se_emit(SE_TOX_SELF_CONNECTION_STATUS, status);

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
                throw toxd_error();
            if(ret == 0 || !(pfd.revents & POLLIN))
                break;
            vc msg = read_msg(fd);
            if(is_event(msg)) {
                process_tox_event(msg);
                ++count;
            } else if(is_response(msg)) {
                if(!Pending_responses.is_nil())
                    Pending_responses.append(msg);
            }
        }
    } catch(toxd_error&) {
        handle_toxd_crash();
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
            throw toxd_error();
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
        if(!Pending_responses.is_nil())
            Pending_responses.append(msg);
    }
    return 0;
}

static int
toxd_rpc_call(const vc &method, const vc &params, vc &result_out, int timeout_ms)
{
    try {
        if(Toxd_stdin < 0)
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
        handle_toxd_crash();
        return 0;
    }
}

int
tox_bridge_init(const char *toxd_path, const char *data_dir)
{
    if(Active)
        return 1;

    if(access(toxd_path, X_OK) != 0) {
        GRTLOG("tox bridge: toxd not found at ", 0, 0);
        GRTLOG(toxd_path, 0, 0);
        return 0;
    }

    int stdin_pipe[2];
    int stdout_pipe[2];

    if(pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0)
        return 0;

    Toxd_pid = fork();
    if(Toxd_pid < 0) {
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        return 0;
    }

    if(Toxd_pid == 0) {
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);

        if(data_dir)
            setenv("DWYCO_TOX_DATA", data_dir, 1);

        execl(toxd_path, toxd_path, (char *)NULL);
        _exit(1);
    }

    close(stdin_pipe[0]);
    close(stdout_pipe[1]);
    Toxd_stdin = stdin_pipe[1];
    Toxd_stdout = stdout_pipe[0];
    Active = 1;

    GRTLOG("tox bridge: initialized, pid=", Toxd_pid, 0);
    return 1;
}

void
tox_bridge_shutdown()
{
    if(!Active)
        return;

    vc result;
    toxd_rpc_call(vc("shutdown"), vc(VC_MAP, "", 0), result, 3000);

    if(Toxd_stdin >= 0) {
        close(Toxd_stdin);
        Toxd_stdin = -1;
    }
    if(Toxd_stdout >= 0) {
        close(Toxd_stdout);
        Toxd_stdout = -1;
    }

    if(Toxd_pid > 0) {
        int status;
        waitpid(Toxd_pid, &status, WNOHANG);
        Toxd_pid = 0;
    }

    Active = 0;
    Pending_responses = vcnil;
    GRTLOG("tox bridge: shutdown", 0, 0);
}

int
tox_bridge_is_active()
{
    return Active;
}

void
tox_bridge_poll()
{
    if(!Active)
        return;
    read_pending_events(Toxd_stdout);
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
    return toxd_rpc_call(vc("friend_add"), params, result, 10000);
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
tox_bridge_send_message(uint32_t friend_number, const vc &text, int is_action)
{
    vc params(VC_MAP, "", 6);
    params.add_kv("friend_number", vc((int)friend_number));
    params.add_kv("text", text);
    params.add_kv("type", vc(is_action ? "action" : "normal"));
    vc result;
    return toxd_rpc_call(vc("message_send"), params, result, 10000);
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
    (void)pseudo_uid;
    (void)fn_out;
    return 0;
}

int
tox_friend_number_to_pseudo_uid(uint32_t fn, vc &pseudo_uid_out)
{
    (void)fn;
    pseudo_uid_out = vcnil;
    return 0;
}

static vc Friend_cache;

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
        GRTLOG("tox bridge: rebuilt friend cache, count=", Friend_cache.num_elems(), 0);
    }
}

int
tox_bridge_is_tox_uid(const vc &uid)
{
    if(Friend_cache.is_nil())
        tox_bridge_rebuild_friend_cache();
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
tox_bridge_get_self_public_key(const char **out, int *len_out)
{
    vc pk = tox_bridge_get_pubkey();
    if(pk.is_nil())
        return 0;
    if(out)
        *out = (const char *)pk;
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

}

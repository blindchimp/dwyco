#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <tox/tox.h>
#include <sodium.h>
#include <signal.h>
#include <vector>

#include "vc.h"
#include "vccomp.h"
#include "vcxstrm.h"
#include "vcio.h"
#include "dwstr.h"

#define RPCM_METHOD 0
#define RPCM_PARAMS 1
#define RPCM_REQID 2

#define RPCM_RES_REQID 0
#define RPCM_RESULT 1

#define EV_TYPE 0
#define EV_ARGS 1

#define DATA_DIR_ENV "DWYCO_TOX_DATA"
#define DEFAULT_DATA_DIR ".config/dwyco/tox"
#define SAVE_FILE "tox_save.tox"
#define LOCK_FILE "toxd.lock"

static const char *bootstrap_nodes[] =
{
    "144.217.167.73:33445:7E5668E0EE09E19F320AD47902419331FFEE147BB3606769CFBE921A2A2FD34C",
    "tox.abilinski.com:33445:10C00EB250C3233E343E2AEBA07115A5C28920E9C8D29492F6D00B29049EDC7E",
    "198.199.98.108:33445:BEF0CFB37AF874BD17B9A8F9FE64C75521DB95A37D33C5BDB00E9CF58659C04F",
    "tox.kurnevsky.net:33445:82EF82BA33445A1F91A7DB27189ECFC0C013E06E3DA71F588ED692BED625EC23",
    "205.185.115.131:53:3091C6BEB2A993F1C6300C16549FABA67098FF3D62C6D253828B531470B53D68",
    "46.101.197.175:33445:CD133B521159541FB1D326DE9850F5E56A6C724B5B8E5EB5CD8D950408E95707",
    "tox.initramfs.io:33445:3F0A45A268367C1BEA652F258C85F4A66DA76BCAA667A49E770BCC4917AB6A25",
    "tox.plastiras.org:33445:8E8B63299B3D520FB377FE5100E65E3322F7AE5B20A0ACED2981769FC5B43725",
    "tox.hidemybits.com:443:5D57B95EE4A7F37BA031DAD0CBD9510A9C96FFE09C1CE24A9C33746F39817D6E",
    "tox.libre.tw:33445:1CEEA650D5DDA858EA6AF6CEA79FEAF022F9C2B8295EE3716E2785C81DD09152",
};

struct ToxdState
{
    Tox *tox;
    int shutdown;
    int reqid_counter;
    char data_dir[1024];
    FILE *log_file;
    int bootstrapped;
};

FILE *Lf;

struct toxd_error
{
    DwString error;
    toxd_error(const DwString& e) : error(e) {}
};

[[noreturn]]
void
oopanic(const char *a)
{
    perror(a);
    ::abort();
}

[[noreturn]]
static void
watchdog_handler(int)
{
    _exit(1);
}

static void
log_printf(FILE *lf, const char *fmt, ...)
{
    if(!lf)
        return;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(lf, fmt, ap);
    va_end(ap);
    fputc('\n', lf);
    fflush(lf);
}

static int
read_all(int fd, char *buf, int len)
{
    int total = 0;
    while(total < len)
    {
        int n = (int)read(fd, buf + total, (size_t)(len - total));
        log_printf(Lf, "read n=%d\n", n);
        fwrite(buf + total, n, 1, Lf);
        fflush(Lf);

        if(n <= 0)
        {
            if(n == 0)
                return total;
            if(errno == EINTR)
                continue;
            throw toxd_error(strerror(errno));
        }
        total += n;
    }
    return total;
}

static int
write_all(int fd, const char *buf, int len)
{
    int total = 0;
    while(total < len)
    {
        int n = (int)write(fd, buf + total, (size_t)(len - total));
        log_printf(Lf, "write n= %d\n", n);
        fwrite(buf + total, n, 1, Lf);
        fflush(Lf);
        if(n <= 0)
        {
            if(errno == EINTR)
                continue;
            throw toxd_error(strerror(errno));
        }
        total += n;
    }
    return total;
}

static void
write_msg(int fd, vc msg)
{
    vcxstream os(0, 2048, vcxstream::CONTINUOUS);
    if(!os.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
        throw toxd_error("open serial");
    vc_composite::new_dfs();
    if(msg.xfer_out(os) < 0)
    {
        os.close(vcxstream::DISCARD);
        throw toxd_error("xfer_out");
    }
    //if(!os.close(vcxstream::FLUSH))
    //    return;
    const char *buf;
    long len;
    os.cur_buf(buf, len);
    uint32_t nlen = htonl((uint32_t)len);
    write_all(fd, (const char *)&nlen, 4);
    write_all(fd, buf, len);
}

static vc
read_msg(int fd)
{
    uint32_t nlen;
    if(read_all(fd, (char *)&nlen, 4) != 4)
        return vcnil;
    long len = (long)ntohl(nlen);
    if(len <= 2 || len > 1024)
        return vcnil;
    char *buf = new char[len];
    if(read_all(fd, buf, (int)len) != (int)len)
    {
        delete[] buf;
        return vcnil;
    }
    vcxstream is(buf, len, vcxstream::FIXED);
    if(!is.open(vcxstream::READABLE))
    {
        delete[] buf;
        throw toxd_error("open stream in");
    }
    vc msg;
    if(msg.xfer_in(is) < 0)
    {
        delete[] buf;
        throw toxd_error("xfer_in");
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

static void
send_response(int fd, int reqid, const vc &result)
{
    vc resp(VC_VECTOR);
    resp.append(vc(reqid));
    resp.append(result);
    write_msg(fd, resp);
}

static void
send_error(int fd, int reqid, const char *err)
{
    vc resp(VC_VECTOR);
    resp.append(vc(reqid));
    resp.append(vc(err));
    write_msg(fd, resp);
}

static void
send_event(int fd, const char *type, const vc &args)
{
    vc ev(VC_VECTOR);
    ev.append(vc(type));
    ev.append(args);
    write_msg(fd, ev);
}

static void
send_simple_event(int fd, const char *type, int a0)
{
    vc args(VC_VECTOR);
    args.append(vc(a0));
    send_event(fd, type, args);
}

static void
tox_bootstrap(Tox *tox)
{
    int n = sizeof(bootstrap_nodes) / sizeof(bootstrap_nodes[0]);
    for(int i = 0; i < n; ++i)
    {
        const char *s = bootstrap_nodes[i];
        char addr[256];
        int port;
        char pubkey_hex[2 * TOX_PUBLIC_KEY_SIZE + 1];
        if(sscanf(s, "%255[^:]:%d:%64s", addr, &port, pubkey_hex) != 3)
            continue;
        uint8_t pubkey[TOX_PUBLIC_KEY_SIZE];
        if(sodium_hex2bin(pubkey, sizeof(pubkey), pubkey_hex, strlen(pubkey_hex), NULL, NULL, NULL))
            continue;
        Tox_Err_Bootstrap err;
        tox_bootstrap(tox, addr, (uint16_t)port, pubkey, &err);
    }
}

static vc
pubkey_to_vc(const uint8_t *pk)
{
    return vc(VC_BSTRING, (const char *)pk, TOX_PUBLIC_KEY_SIZE);
}


static void
log_cmd(ToxdState *s, const char *method, int reqid,  vc params)
{
    if(!s || !s->log_file)
        return;
    fprintf(s->log_file, "[cmd] %s reqid=%d", method, reqid);
    if(!params.is_nil() && params.num_elems() > 0)
    {
        VcIOHackStr buf;
        params.printOn(buf);
        fprintf(s->log_file, " params=%.*s", buf.pcount(), buf.ref_str());
    }
    fputc('\n', s->log_file);
    fflush(s->log_file);
}

static void
toxd_on_friend_request(Tox *tox, const uint8_t *pubkey, const uint8_t *msg,
                       size_t len, void *ud)
{
    (void)tox;
    ToxdState *s = (ToxdState *)ud;
    log_printf(s->log_file, "[cb] friend_request msg=%.*s", (int)len, (const char *)msg);
    vc args(VC_VECTOR);
    args.append(pubkey_to_vc(pubkey));
    args.append(vc(VC_STRING, (const char *)msg, (long)len));
    send_event(STDOUT_FILENO, "friend_request", args);
}

static void
toxd_on_friend_message(Tox *tox, uint32_t fn, Tox_Message_Type type,
                       const uint8_t *msg, size_t len, void *ud)
{
    (void)tox;
    ToxdState *s = (ToxdState *)ud;
    log_printf(s->log_file, "[cb] friend_message fn=%u type=%s msg=%.*s", fn,
               type == TOX_MESSAGE_TYPE_ACTION ? "action" : "normal",
               (int)len, (const char *)msg);
    uint8_t pubkey[TOX_PUBLIC_KEY_SIZE];
    tox_friend_get_public_key(tox, fn, pubkey, NULL);
    vc args(VC_VECTOR);
    args.append(vc((int)fn));
    args.append(pubkey_to_vc(pubkey));
    args.append(vc(VC_STRING, (const char *)msg, (long)len));
    args.append(vc(type == TOX_MESSAGE_TYPE_ACTION ? "action" : "normal"));
    send_event(STDOUT_FILENO, "message", args);
}

static void
toxd_on_friend_read_receipt(Tox *tox, uint32_t fn, uint32_t mid, void *ud)
{
    (void)tox;
    ToxdState *s = (ToxdState *)ud;
    log_printf(s->log_file, "[cb] read_receipt fn=%u mid=%u", fn, mid);
    vc args(VC_VECTOR);
    args.append(vc((int)fn));
    args.append(vc((int)mid));
    send_event(STDOUT_FILENO, "read_receipt", args);
}

static void
toxd_on_file_recv(Tox *tox, uint32_t fn, uint32_t fnum, uint32_t kind,
                  uint64_t size, const uint8_t *name, size_t nlen, void *ud)
{
    (void)tox;
    ToxdState *s = (ToxdState *)ud;
    log_printf(s->log_file, "[cb] file_request fn=%u fnum=%u kind=%u size=%llu name=%.*s",
               fn, fnum, kind, (unsigned long long)size, (int)nlen, (const char *)name);
    vc args(VC_VECTOR);
    args.append(vc((int)fn));
    args.append(vc((int)fnum));
    args.append(vc((int)kind));
    args.append(vc((long long)size));
    if(name && nlen > 0)
        args.append(vc(VC_BSTRING, (const char *)name, (long)nlen));
    else
        args.append(vcnil);
    send_event(STDOUT_FILENO, "file_request", args);
}

static void
toxd_on_file_recv_chunk(Tox *tox, uint32_t fn, uint32_t fnum,
                        uint64_t pos, const uint8_t *data, size_t len, void *ud)
{
    (void)tox;
    ToxdState *s = (ToxdState *)ud;
    log_printf(s->log_file, "[cb] file_chunk fn=%u fnum=%u pos=%llu len=%zu",
               fn, fnum, (unsigned long long)pos, len);
    vc args(VC_VECTOR);
    args.append(vc((int)fn));
    args.append(vc((int)fnum));
    args.append(vc((long long)pos));
    if(data && len)
    {
        args.append(vc(VC_BSTRING, (const char *)data, (long)len));
    }
    else
    {
        args.append(vcnil);
    }
    send_event(STDOUT_FILENO, "file_chunk", args);
}

static void
toxd_on_self_connection_status(Tox *tox, Tox_Connection status, void *ud)
{
    ToxdState *s = (ToxdState *)ud;
    log_printf(s->log_file, "[cb] self_connection_status %s",
               status == TOX_CONNECTION_NONE ? "offline" : status == TOX_CONNECTION_TCP ? "tcp" : "udp");
    vc args(VC_VECTOR);
    args.append(vc(status == TOX_CONNECTION_NONE ? "offline" : status == TOX_CONNECTION_TCP ? "tcp" : "udp"));
    send_event(STDOUT_FILENO, "self_connection_status", args);
}

static void
toxd_on_connection_status(Tox *tox, uint32_t fn, Tox_Connection status, void *ud)
{
    ToxdState *s = (ToxdState *)ud;
    log_printf(s->log_file, "[cb] friend_connection_status fn=%u %s", fn,
               status == TOX_CONNECTION_NONE ? "offline" : "online");
    uint8_t pubkey[TOX_PUBLIC_KEY_SIZE];
    tox_friend_get_public_key(tox, fn, pubkey, NULL);
    vc args(VC_VECTOR);
    args.append(vc((int)fn));
    args.append(pubkey_to_vc(pubkey));
    args.append(vc(status == TOX_CONNECTION_NONE ? "offline" : "online"));
    send_event(STDOUT_FILENO, "friend_connection_status", args);
}

static void
toxd_on_friend_name(Tox *tox, uint32_t fn, const uint8_t *name, size_t len, void *ud)
{
    ToxdState *s = (ToxdState *)ud;
    log_printf(s->log_file, "[cb] friend_name fn=%u name=%.*s", fn, (int)len, (const char *)name);
    uint8_t pubkey[TOX_PUBLIC_KEY_SIZE];
    tox_friend_get_public_key(tox, fn, pubkey, NULL);
    vc args(VC_VECTOR);
    args.append(vc((int)fn));
    args.append(pubkey_to_vc(pubkey));
    args.append(vc(VC_STRING, (const char *)name, (long)len));
    send_event(STDOUT_FILENO, "friend_name", args);
}

static void
toxd_on_friend_status_message(Tox *tox, uint32_t fn, const uint8_t *msg,
                              size_t len, void *ud)
{
    ToxdState *s = (ToxdState *)ud;
    log_printf(s->log_file, "[cb] friend_status_message fn=%u msg=%.*s", fn, (int)len, (const char *)msg);
    uint8_t pubkey[TOX_PUBLIC_KEY_SIZE];
    tox_friend_get_public_key(tox, fn, pubkey, NULL);
    vc args(VC_VECTOR);
    args.append(vc((int)fn));
    args.append(pubkey_to_vc(pubkey));
    args.append(vc(VC_STRING, (const char *)msg, (long)len));
    send_event(STDOUT_FILENO, "friend_status_message", args);
}

static void
toxd_on_friend_typing(Tox *tox, uint32_t fn, bool typing, void *ud)
{
    (void)tox;
    ToxdState *s = (ToxdState *)ud;
    log_printf(s->log_file, "[cb] friend_typing fn=%u typing=%d", fn, typing);
    uint8_t pubkey[TOX_PUBLIC_KEY_SIZE];
    tox_friend_get_public_key(tox, fn, pubkey, NULL);
    vc args(VC_VECTOR);
    args.append(vc((int)fn));
    args.append(pubkey_to_vc(pubkey));
    args.append(vc(typing ? "on" : "off"));
    send_event(STDOUT_FILENO, "typing", args);
}

static void
save_tox_state(ToxdState *s)
{
    char save_path[1024];
    snprintf(save_path, sizeof(save_path), "%s/%s", s->data_dir, SAVE_FILE);
    char tmp_path[1024];
    snprintf(tmp_path, sizeof(tmp_path), "%s/%s.new", s->data_dir, SAVE_FILE);

    size_t sz = tox_get_savedata_size(s->tox);
    uint8_t *data = (uint8_t *)malloc(sz);
    tox_get_savedata(s->tox, data);

    FILE *f = fopen(tmp_path, "wb");
    if(f)
    {
        fwrite(data, 1, sz, f);
        fclose(f);
        rename(tmp_path, save_path);
    }
    free(data);
}

static void
tox_write_log(Tox *tox, Tox_Log_Level level, const char *file,
              uint32_t line, const char *func, const char *message,
              void *user_data)
{
    FILE *lf = (FILE *)user_data;
    if(lf)
    {
        fprintf(lf, "[%s] %s:%u (%s): %s\n",
                tox_log_level_to_string(level), file, line, func, message);
        fflush(lf);
    }
}

static void
load_or_create_tox(ToxdState *s)
{
    char save_path[1024];
    snprintf(save_path, sizeof(save_path), "%s/%s", s->data_dir, SAVE_FILE);

    struct Tox_Options opts;
    memset(&opts, 0, sizeof(opts));
    tox_options_default(&opts);

    const char *home = getenv("HOME");
    char log_path[1024];
    if(home)
        snprintf(log_path, sizeof(log_path), "%s/tox.log", home);
    else
        snprintf(log_path, sizeof(log_path), "tox.log");
    Lf = s->log_file = fopen(log_path, "a");
    if(s->log_file)
    {
        opts.log_callback = tox_write_log;
        opts.log_user_data = s->log_file;
    }

    uint8_t *savedata = NULL;
    size_t savedata_sz = 0;
    FILE *f = fopen(save_path, "rb");
    if(f)
    {
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        rewind(f);
        if(sz > 0 && sz < 10 * 1024 * 1024)
        {
            savedata = (uint8_t *)malloc((size_t)sz);
            if(fread(savedata, 1, (size_t)sz, f) == (size_t)sz)
            {
                savedata_sz = (size_t)sz;
            }
            else
            {
                free(savedata);
                savedata = NULL;
            }
        }
        fclose(f);
    }

    if(savedata)
    {
        opts.savedata_type = TOX_SAVEDATA_TYPE_TOX_SAVE;
        opts.savedata_data = savedata;
        opts.savedata_length = savedata_sz;
    }

    Tox_Err_New err;
    s->tox = tox_new(&opts, &err);
    if(savedata)
        free(savedata);
    if(!s->tox)
    {
        tox_options_default(&opts);
        opts.savedata_type = TOX_SAVEDATA_TYPE_NONE;
        s->tox = tox_new(&opts, &err);
    }

    if(s->tox)
    {
        uint8_t address[TOX_ADDRESS_SIZE];
        tox_self_get_address(s->tox, address);
        char hex[TOX_ADDRESS_SIZE * 2 + 1];
        sodium_bin2hex(hex, sizeof(hex), address, TOX_ADDRESS_SIZE);
        fprintf(stderr, "toxd: %s %s\n", savedata ? "loaded" : "created", hex);

        if(!savedata)
            save_tox_state(s);
    }
}

static void
register_callbacks(Tox *tox)
{
    tox_callback_friend_request(tox, toxd_on_friend_request);
    tox_callback_friend_message(tox, toxd_on_friend_message);
    tox_callback_friend_read_receipt(tox, toxd_on_friend_read_receipt);
    tox_callback_file_recv(tox, toxd_on_file_recv);
    tox_callback_file_recv_chunk(tox, toxd_on_file_recv_chunk);
    tox_callback_friend_connection_status(tox, toxd_on_connection_status);
    tox_callback_self_connection_status(tox, toxd_on_self_connection_status);
    tox_callback_friend_name(tox, toxd_on_friend_name);
    tox_callback_friend_status_message(tox, toxd_on_friend_status_message);
    tox_callback_friend_typing(tox, toxd_on_friend_typing);
}

static void
handle_generate_keypair(ToxdState *s, vc params, int reqid)
{
    log_cmd(s, "generate_keypair", reqid, params);
    uint8_t pubkey[TOX_PUBLIC_KEY_SIZE];
    tox_self_get_public_key(s->tox, pubkey);
    uint8_t address[TOX_ADDRESS_SIZE];
    tox_self_get_address(s->tox, address);

    vc result(VC_MAP, "", 4);
    result.add_kv("pubkey", vc(VC_BSTRING, (const char *)pubkey, TOX_PUBLIC_KEY_SIZE));
    result.add_kv("address", vc(VC_BSTRING, (const char *)address, TOX_ADDRESS_SIZE));
    send_response(STDOUT_FILENO, reqid, result);
}

static void
handle_get_address(ToxdState *s, vc params, int reqid)
{
    log_cmd(s, "get_address", reqid, params);
    uint8_t address[TOX_ADDRESS_SIZE];
    tox_self_get_address(s->tox, address);

    vc result(VC_MAP, "", 2);
    result.add_kv("address", vc(VC_BSTRING, (const char *)address, TOX_ADDRESS_SIZE));
    send_response(STDOUT_FILENO, reqid, result);
}

static void
handle_friend_add(ToxdState *s, vc params, int reqid)
{
    log_cmd(s, "friend_add", reqid, params);
    vc addr_vc;
    vc msg_vc;
    if(!params.find("address", addr_vc) || !params.find("message", msg_vc))
    {
        send_error(STDOUT_FILENO, reqid, "missing address or message");
        return;
    }
    if(addr_vc.len() != TOX_ADDRESS_SIZE)
    {
        send_error(STDOUT_FILENO, reqid, "invalid address length");
        return;
    }

    uint8_t addr[TOX_ADDRESS_SIZE];
    memcpy(addr, (const char *)addr_vc, TOX_ADDRESS_SIZE);
    const uint8_t *msg = (const uint8_t *)(const char *)msg_vc;
    size_t mlen = (size_t)msg_vc.len();

    Tox_Err_Friend_Add err;
    uint32_t fn = tox_friend_add(s->tox, addr, msg, mlen, &err);
    if(err != TOX_ERR_FRIEND_ADD_OK)
    {
        const char *estr = "unknown error";
        switch(err)
        {
        case TOX_ERR_FRIEND_ADD_TOO_LONG:
            estr = "message too long";
            break;
        case TOX_ERR_FRIEND_ADD_NO_MESSAGE:
            estr = "no message";
            break;
        case TOX_ERR_FRIEND_ADD_OWN_KEY:
            estr = "own key";
            break;
        case TOX_ERR_FRIEND_ADD_ALREADY_SENT:
            estr = "already sent";
            break;
        case TOX_ERR_FRIEND_ADD_BAD_CHECKSUM:
            estr = "bad checksum";
            break;
        case TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM:
            estr = "set new nospam";
            break;
        case TOX_ERR_FRIEND_ADD_MALLOC:
            estr = "malloc failed";
            break;
        default:
            break;
        }
        send_error(STDOUT_FILENO, reqid, estr);
        return;
    }

    vc result(VC_MAP, "", 2);
    result.add_kv("friend_number", vc((int)fn));
    send_response(STDOUT_FILENO, reqid, result);
    save_tox_state(s);
}

static void
handle_friend_add_norequest(ToxdState *s, vc params, int reqid)
{
    log_cmd(s, "friend_add_norequest", reqid, params);
    vc pk_vc;
    if(!params.find("pubkey", pk_vc))
    {
        send_error(STDOUT_FILENO, reqid, "missing pubkey");
        return;
    }
    if(pk_vc.len() != TOX_PUBLIC_KEY_SIZE)
    {
        send_error(STDOUT_FILENO, reqid, "invalid pubkey length");
        return;
    }

    Tox_Public_Key pk;
    memcpy(pk, (const char *)pk_vc, TOX_PUBLIC_KEY_SIZE);
    Tox_Err_Friend_Add err;
    uint32_t fn = tox_friend_add_norequest(s->tox, pk, &err);
    if(err != TOX_ERR_FRIEND_ADD_OK)
    {
        const char *estr = "unknown error";
        switch(err)
        {
        case TOX_ERR_FRIEND_ADD_TOO_LONG:
            estr = "message too long";
            break;
        case TOX_ERR_FRIEND_ADD_NO_MESSAGE:
            estr = "no message";
            break;
        case TOX_ERR_FRIEND_ADD_OWN_KEY:
            estr = "own key";
            break;
        case TOX_ERR_FRIEND_ADD_ALREADY_SENT:
            estr = "already sent";
            break;
        case TOX_ERR_FRIEND_ADD_BAD_CHECKSUM:
            estr = "bad checksum";
            break;
        case TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM:
            estr = "set new nospam";
            break;
        case TOX_ERR_FRIEND_ADD_MALLOC:
            estr = "malloc failed";
            break;
        default:
            break;
        }
        send_error(STDOUT_FILENO, reqid, estr);
        return;
    }

    vc result(VC_MAP, "", 2);
    result.add_kv("friend_number", vc((int)fn));
    send_response(STDOUT_FILENO, reqid, result);
    save_tox_state(s);
}

static void
handle_friend_delete(ToxdState *s, vc params, int reqid)
{
    log_cmd(s, "friend_delete", reqid, params);
    vc fn_vc;
    if(!params.find("friend_number", fn_vc))
    {
        send_error(STDOUT_FILENO, reqid, "missing friend_number");
        return;
    }
    uint32_t fn = (uint32_t)(int)fn_vc;
    Tox_Err_Friend_Delete err;
    tox_friend_delete(s->tox, fn, &err);
    if(err != TOX_ERR_FRIEND_DELETE_OK)
    {
        send_error(STDOUT_FILENO, reqid, "friend not found");
        return;
    }
    send_response(STDOUT_FILENO, reqid, vcnil);
    save_tox_state(s);
}

static void
handle_message_send(ToxdState *s, vc params, int reqid)
{
    log_cmd(s, "message_send", reqid, params);
    vc fn_vc, text_vc, type_vc;
    if(!params.find("friend_number", fn_vc) || !params.find("text", text_vc))
    {
        send_error(STDOUT_FILENO, reqid, "missing friend_number or text");
        return;
    }
    uint32_t fn = (uint32_t)(int)fn_vc;
    Tox_Message_Type mtype = TOX_MESSAGE_TYPE_NORMAL;
    if(!params.find("type", type_vc))
    {
        if(type_vc == vc("action"))
            mtype = TOX_MESSAGE_TYPE_ACTION;
    }

    const uint8_t *msg = (const uint8_t *)(const char *)text_vc;
    size_t mlen = (size_t)text_vc.len();

    Tox_Err_Friend_Send_Message err;
    uint32_t mid = tox_friend_send_message(s->tox, fn, mtype, msg, mlen, &err);
    if(err != TOX_ERR_FRIEND_SEND_MESSAGE_OK)
    {
        vc result(VC_MAP, "", 2);
        result.add_kv("tox_error", vc((int)err));
        send_response(STDOUT_FILENO, reqid, result);
        return;
    }

    vc result(VC_MAP, "", 2);
    result.add_kv("message_id", vc((int)mid));
    send_response(STDOUT_FILENO, reqid, result);
}

static void
handle_typing_set(ToxdState *s, vc params, int reqid)
{
    log_cmd(s, "typing_set", reqid, params);
    vc fn_vc, typing_vc;
    if(!params.find("friend_number", fn_vc) || !params.find("typing", typing_vc))
    {
        send_error(STDOUT_FILENO, reqid, "missing friend_number or typing");
        return;
    }
    uint32_t fn = (uint32_t)(int)fn_vc;
    bool typing = ((int)typing_vc) != 0;
    Tox_Err_Set_Typing err;
    if(!tox_self_set_typing(s->tox, fn, typing, &err))
    {
        send_error(STDOUT_FILENO, reqid, "typing set failed");
        return;
    }
    send_response(STDOUT_FILENO, reqid, vcnil);
}

static void
handle_file_send(ToxdState *s, vc params, int reqid)
{
    log_cmd(s, "file_send", reqid, params);
    vc fn_vc, name_vc;
    vc size_vc;
    if(!params.find("friend_number", fn_vc) || !params.find("name", name_vc) ||
            !params.find("size", size_vc))
    {
        send_error(STDOUT_FILENO, reqid, "missing friend_number, name, or size");
        return;
    }
    uint32_t fn = (uint32_t)(int)fn_vc;
    uint32_t kind = TOX_FILE_KIND_DATA;
    vc kind_vc;
    if(params.find("kind", kind_vc))
        kind = (uint32_t)(int)kind_vc;

    const uint8_t *fname = (const uint8_t *)(const char *)name_vc;
    size_t fnlen = (size_t)name_vc.len();
    uint64_t fsize = 0;
    if(size_vc.type() == VC_INT)
        fsize = (uint64_t)(long long)size_vc;

    Tox_Err_File_Send err;
    uint32_t fnum = tox_file_send(s->tox, fn, kind, fsize, NULL, fname, fnlen, &err);
    if(err != TOX_ERR_FILE_SEND_OK)
    {
        send_error(STDOUT_FILENO, reqid, "file send failed");
        return;
    }

    vc result(VC_MAP, "", 2);
    result.add_kv("file_number", vc((int)fnum));
    send_response(STDOUT_FILENO, reqid, result);
}

static void
handle_file_send_data(ToxdState *s, vc params, int reqid)
{
    log_cmd(s, "file_send_data", reqid, params);
    vc fn_vc, fnum_vc, pos_vc, data_vc;
    if(!params.find("friend_number", fn_vc) || !params.find("file_number", fnum_vc) ||
            !params.find("pos", pos_vc) || !params.find("data", data_vc))
    {
        send_error(STDOUT_FILENO, reqid, "missing required params");
        return;
    }
    uint32_t fn = (uint32_t)(int)fn_vc;
    uint32_t fnum = (uint32_t)(int)fnum_vc;
    uint64_t pos = 0;
    if(pos_vc.type() == VC_INT)
        pos = (uint64_t)(long long)pos_vc;

    Tox_Err_File_Send_Chunk err;
    if(data_vc.is_nil() || data_vc.len() == 0)
    {
        tox_file_send_chunk(s->tox, fn, fnum, pos, NULL, 0, &err);
    }
    else
    {
        const uint8_t *data = (const uint8_t *)(const char *)data_vc;
        size_t dlen = (size_t)data_vc.len();
        tox_file_send_chunk(s->tox, fn, fnum, pos, data, dlen, &err);
    }
    if(err != TOX_ERR_FILE_SEND_CHUNK_OK)
    {
        send_error(STDOUT_FILENO, reqid, "file chunk send failed");
        return;
    }
    send_response(STDOUT_FILENO, reqid, vcnil);
}

static void
handle_file_accept(ToxdState *s, vc params, int reqid)
{
    log_cmd(s, "file_accept", reqid, params);
    vc fn_vc, fnum_vc;
    if(!params.find("friend_number", fn_vc) || !params.find("file_number", fnum_vc))
    {
        send_error(STDOUT_FILENO, reqid, "missing friend_number or file_number");
        return;
    }
    uint32_t fn = (uint32_t)(int)fn_vc;
    uint32_t fnum = (uint32_t)(int)fnum_vc;

    Tox_Err_File_Control err;
    tox_file_control(s->tox, fn, fnum, TOX_FILE_CONTROL_RESUME, &err);
    if(err != TOX_ERR_FILE_CONTROL_OK)
    {
        send_error(STDOUT_FILENO, reqid, "file accept failed");
        return;
    }
    send_response(STDOUT_FILENO, reqid, vcnil);
}

static void
handle_ping(ToxdState *s, vc params, int reqid)
{
    log_cmd(s, "ping", reqid, params);
    vc result(VC_MAP, "", 2);
    result.add_kv("pong", vc(1));
    send_response(STDOUT_FILENO, reqid, result);
}

static void
handle_friend_list(ToxdState *s, vc params, int reqid)
{
    log_cmd(s, "friend_list", reqid, params);
    uint32_t n = tox_self_get_friend_list_size(s->tox);
    std::vector<uint32_t> friend_list(n);
    tox_self_get_friend_list(s->tox, friend_list.data());
    vc friends(VC_VECTOR);
    for(uint32_t i = 0; i < n; ++i)
    {
        uint32_t fn = friend_list[i];
        uint8_t pubkey[TOX_PUBLIC_KEY_SIZE];
        if(tox_friend_get_public_key(s->tox, fn, pubkey, NULL))
        {
            vc entry(VC_MAP, "", 8);
            entry.add_kv("friend_number", vc((int)fn));
            entry.add_kv("pubkey", vc(VC_BSTRING, (const char *)pubkey, TOX_PUBLIC_KEY_SIZE));

            size_t name_len = tox_friend_get_name_size(s->tox, fn, NULL);
            if(name_len > 0 && name_len <= TOX_MAX_NAME_LENGTH)
            {
                uint8_t name[TOX_MAX_NAME_LENGTH];
                tox_friend_get_name(s->tox, fn, name, NULL);
                entry.add_kv("name", vc(VC_BSTRING, (const char *)name, (long)name_len));
            }
            else
            {
                entry.add_kv("name", vc(""));
            }

            Tox_Connection conn = tox_friend_get_connection_status(s->tox, fn, NULL);
            entry.add_kv("status", vc(conn == TOX_CONNECTION_NONE ? "offline" : "online"));

            friends.append(entry);
        }
    }
    vc result(VC_MAP, "", 2);
    result.add_kv("friends", friends);
    send_response(STDOUT_FILENO, reqid, result);
}

static void
handle_shutdown(ToxdState *s, vc params, int reqid)
{
    log_cmd(s, "shutdown", reqid, params);
    save_tox_state(s);
    send_response(STDOUT_FILENO, reqid, vcnil);
    s->shutdown = 1;
}

static void
handle_rpc_request(ToxdState *s, const vc &req)
{
    vc method_vc = req[RPCM_METHOD];
    vc params = req[RPCM_PARAMS];
    int reqid = (int)req[RPCM_REQID];
    const char *method = (const char *)method_vc;

    log_printf(s->log_file, "[cmd] request method=%s reqid=%d", method, reqid);

    if(strcmp(method, "generate_keypair") == 0)
        handle_generate_keypair(s, params, reqid);
    else if(strcmp(method, "get_address") == 0)
        handle_get_address(s, params, reqid);
    else if(strcmp(method, "friend_add") == 0)
        handle_friend_add(s, params, reqid);
    else if(strcmp(method, "friend_add_norequest") == 0)
        handle_friend_add_norequest(s, params, reqid);
    else if(strcmp(method, "friend_delete") == 0)
        handle_friend_delete(s, params, reqid);
    else if(strcmp(method, "message_send") == 0)
        handle_message_send(s, params, reqid);
    else if(strcmp(method, "typing_set") == 0)
        handle_typing_set(s, params, reqid);
    else if(strcmp(method, "file_send") == 0)
        handle_file_send(s, params, reqid);
    else if(strcmp(method, "file_send_data") == 0)
        handle_file_send_data(s, params, reqid);
    else if(strcmp(method, "file_accept") == 0)
        handle_file_accept(s, params, reqid);
    else if(strcmp(method, "friend_list") == 0)
        handle_friend_list(s, params, reqid);
    else if(strcmp(method, "ping") == 0)
        handle_ping(s, params, reqid);
    else if(strcmp(method, "shutdown") == 0)
        handle_shutdown(s, params, reqid);
    else
        send_error(STDOUT_FILENO, reqid, "unknown method");
}

static int
ensure_parent_dirs(char *path)
{
    for(char *p = path + 1; *p; p++)
    {
        if(*p == '/')
        {
            *p = '\0';
            if(mkdir(path, 0700) < 0 && errno != EEXIST)
                return 0;
            *p = '/';
        }
    }
    return 1;
}

static int
ensure_data_dir(const char *dir)
{
    struct stat st;
    if(stat(dir, &st) == 0)
        return S_ISDIR(st.st_mode);

    char *p = strdup(dir);
    if(!p)
        return 0;
    int ok = ensure_parent_dirs(p) && (mkdir(p, 0700) == 0 || errno == EEXIST);
    free(p);
    return ok;
}

int
main(int argc, char **argv)
{
    ToxdState state;
    memset(&state, 0, sizeof(state));

    const char *data_dir = getenv(DATA_DIR_ENV);
    if(!data_dir)
    {
        const char *home = getenv("HOME");
        if(home)
            snprintf(state.data_dir, sizeof(state.data_dir), "%s/%s", home, DEFAULT_DATA_DIR);
        else
            snprintf(state.data_dir, sizeof(state.data_dir), DEFAULT_DATA_DIR);
    }
    else
    {
        snprintf(state.data_dir, sizeof(state.data_dir), "%s", data_dir);
    }

    if(!ensure_data_dir(state.data_dir))
        return 1;

    char incoming_dir[1024];
    snprintf(incoming_dir, sizeof(incoming_dir), "%s/incoming", state.data_dir);
    ensure_data_dir(incoming_dir);

    char lock_path[1024];
    snprintf(lock_path, sizeof(lock_path), "%s/%s", state.data_dir, LOCK_FILE);
    int lock_fd = open(lock_path, O_CREAT | O_RDWR, 0644);
    if(lock_fd < 0)
        return 1;
    if(flock(lock_fd, LOCK_EX | LOCK_NB) < 0)
    {
        close(lock_fd);
        return 1;
    }

    load_or_create_tox(&state);
    log_printf(state.log_file, "FUCK!");
    if(!state.tox)
        return 1;

    register_callbacks(state.tox);
    tox_self_set_name(state.tox, (const uint8_t *)"test-tox", 9, NULL);

    signal(SIGPIPE, SIG_IGN);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = watchdog_handler;
    sigaction(SIGALRM, &sa, NULL);

    try
    {
        send_event(STDOUT_FILENO, "ready", vc(VC_MAP, "", 0));

        while(!state.shutdown)
        {
            int interval = tox_iteration_interval(state.tox);
            struct pollfd pfd;
            pfd.fd = STDIN_FILENO;
            pfd.events = POLLIN;
            int ret = poll(&pfd, 1, interval < 50 ? 50 : interval);

            if(ret > 0 && (pfd.revents & POLLIN))
            {
                alarm(1);
                vc req = read_msg(STDIN_FILENO);
                if(!req.is_nil())
                {
                    // if(is_response(req))
                    // { alarm(0); continue; }
                    // if(is_event(req))
                    // { alarm(0); continue; }
                    if(req.type() == VC_VECTOR && req.num_elems() >= 3)
                        handle_rpc_request(&state, req);
                }
                else
                {
                    alarm(0); break;
                }
                alarm(0);
            }

            alarm(1);
            tox_iterate(state.tox, &state);
            alarm(0);

            if(!state.bootstrapped)
            {
                tox_bootstrap(state.tox);
                state.bootstrapped = 1;
            }
        }
    }
    catch(const toxd_error& e)
    {
        log_printf(state.log_file, "io error: %s", e.error.c_str());
    }

    log_printf(state.log_file, "DUMP!");


    save_tox_state(&state);
    tox_kill(state.tox);
    close(lock_fd);
    unlink(lock_path);
    if(state.log_file)
        fclose(state.log_file);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#ifdef LOCAL_TOXCORE
#include <tox.h>
#else
#include <tox/tox.h>
#endif
#include <sodium.h>
#include <vector>

#include "vc.h"
#include "vccomp.h"
#include "dwstr.h"
#include "toxd_plugin.h"

// Order: static IP entries, then resolved IP equivalents of hostname entries,
// then hostname entries (dead code since DNS is disabled via
// experimental_disable_dns, kept as documentation).
static const char *bootstrap_nodes[] =
{
    // --- static IP entries ---
    "144.217.167.73:33445:7E5668E0EE09E19F320AD47902419331FFEE147BB3606769CFBE921A2A2FD34C",
    "198.199.98.108:33445:BEF0CFB37AF874BD17B9A8F9FE64C75521DB95A37D33C5BDB00E9CF58659C04F",
    "205.185.115.131:53:3091C6BEB2A993F1C6300C16549FABA67098FF3D62C6D253828B531470B53D68",
    "46.101.197.175:33445:CD133B521159541FB1D326DE9850F5E56A6C724B5B8E5EB5CD8D950408E95707",

    // --- resolved IPs of hostname nodes (used when DNS is disabled) ---
    "24.87.200.32:33445:10C00EB250C3233E343E2AEBA07115A5C28920E9C8D29492F6D00B29049EDC7E",
    "114.35.245.150:33445:3F0A45A268367C1BEA652F258C85F4A66DA76BCAA667A49E770BCC4917AB6A25",
    "104.244.74.69:33445:8E8B63299B3D520FB377FE5100E65E3322F7AE5B20A0ACED2981769FC5B43725",
    "165.227.194.41:443:5D57B95EE4A7F37BA031DAD0CBD9510A9C96FFE09C1CE24A9C33746F39817D6E",
    "90.89.165.222:33445:1CEEA650D5DDA858EA6AF6CEA79FEAF022F9C2B8295EE3716E2785C81DD09152",

    // --- hostname entries (documentation only; DNS disabled, always BAD_HOST) ---
    "tox.abilinski.com:33445:10C00EB250C3233E343E2AEBA07115A5C28920E9C8D29492F6D00B29049EDC7E",
    "tox.initramfs.io:33445:3F0A45A268367C1BEA652F258C85F4A66DA76BCAA667A49E770BCC4917AB6A25",
    "tox.plastiras.org:33445:8E8B63299B3D520FB377FE5100E65E3322F7AE5B20A0ACED2981769FC5B43725",
    "tox.hidemybits.com:443:5D57B95EE4A7F37BA031DAD0CBD9510A9C96FFE09C1CE24A9C33746F39817D6E",
    "tox.libre.tw:33445:1CEEA650D5DDA858EA6AF6CEA79FEAF022F9C2B8295EE3716E2785C81DD09152",
};

struct ToxPlugin
{
    Tox *tox{};
    DwString data_dir{};
    FILE *log_file{};
    int bootstrapped{};
    int lock_fd{-1};
    DwString lock_path;
    ToxpEventCB event_cb{};
    void *event_userdata{};
};

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

static vc
pubkey_to_vc(const uint8_t *pk)
{
    return vc(VC_BSTRING, (const char *)pk, TOX_PUBLIC_KEY_SIZE);
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

// --- toxcore callbacks (forward events to toxbridge) ---

static void
toxd_on_friend_request(Tox *tox, const uint8_t *pubkey, const uint8_t *msg,
                       size_t len, void *ud)
{
    (void)tox;
    ToxPlugin *p = (ToxPlugin *)ud;
    log_printf(p->log_file, "[cb] friend_request msg=%.*s", (int)len, (const char *)msg);
    vc args(VC_VECTOR);
    args.append(pubkey_to_vc(pubkey));
    args.append(vc(VC_STRING, (const char *)msg, (long)len));
    p->event_cb("friend_request", args, p->event_userdata);
}

static void
toxd_on_friend_message(Tox *tox, uint32_t fn, Tox_Message_Type type,
                       const uint8_t *msg, size_t len, void *ud)
{
    (void)tox;
    ToxPlugin *p = (ToxPlugin *)ud;
    log_printf(p->log_file, "[cb] friend_message fn=%u type=%s msg=%.*s", fn,
               type == TOX_MESSAGE_TYPE_ACTION ? "action" : "normal",
               (int)len, (const char *)msg);
    uint8_t pubkey[TOX_PUBLIC_KEY_SIZE];
    tox_friend_get_public_key(tox, fn, pubkey, NULL);
    vc args(VC_VECTOR);
    args.append(vc((int)fn));
    args.append(pubkey_to_vc(pubkey));
    args.append(vc(VC_STRING, (const char *)msg, (long)len));
    args.append(vc(type == TOX_MESSAGE_TYPE_ACTION ? "action" : "normal"));
    p->event_cb("message", args, p->event_userdata);
}

static void
toxd_on_friend_read_receipt(Tox *tox, uint32_t fn, uint32_t mid, void *ud)
{
    (void)tox;
    ToxPlugin *p = (ToxPlugin *)ud;
    log_printf(p->log_file, "[cb] read_receipt fn=%u mid=%u", fn, mid);
    vc args(VC_VECTOR);
    args.append(vc((int)fn));
    args.append(vc((int)mid));
    p->event_cb("read_receipt", args, p->event_userdata);
}

static void
toxd_on_file_recv(Tox *tox, uint32_t fn, uint32_t fnum, uint32_t kind,
                  uint64_t size, const uint8_t *name, size_t nlen, void *ud)
{
    (void)tox;
    ToxPlugin *p = (ToxPlugin *)ud;
    log_printf(p->log_file, "[cb] file_request fn=%u fnum=%u kind=%u size=%llu name=%.*s",
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
    p->event_cb("file_request", args, p->event_userdata);
}

static void
toxd_on_file_recv_chunk(Tox *tox, uint32_t fn, uint32_t fnum,
                        uint64_t pos, const uint8_t *data, size_t len, void *ud)
{
    (void)tox;
    ToxPlugin *p = (ToxPlugin *)ud;
    log_printf(p->log_file, "[cb] file_chunk fn=%u fnum=%u pos=%llu len=%zu",
               fn, fnum, (unsigned long long)pos, len);
    vc args(VC_VECTOR);
    args.append(vc((int)fn));
    args.append(vc((int)fnum));
    args.append(vc((long long)pos));
    if(data && len)
        args.append(vc(VC_BSTRING, (const char *)data, (long)len));
    else
        args.append(vcnil);
    p->event_cb("file_chunk", args, p->event_userdata);
}

static void
toxd_on_self_connection_status(Tox *tox, Tox_Connection status, void *ud)
{
    ToxPlugin *p = (ToxPlugin *)ud;
    log_printf(p->log_file, "[cb] self_connection_status %s",
               status == TOX_CONNECTION_NONE ? "offline" : status == TOX_CONNECTION_TCP ? "tcp" : "udp");
    vc args(VC_VECTOR);
    args.append(vc(status == TOX_CONNECTION_NONE ? "offline" : status == TOX_CONNECTION_TCP ? "tcp" : "udp"));
    p->event_cb("self_connection_status", args, p->event_userdata);
}

static void
toxd_on_connection_status(Tox *tox, uint32_t fn, Tox_Connection status, void *ud)
{
    ToxPlugin *p = (ToxPlugin *)ud;
    log_printf(p->log_file, "[cb] friend_connection_status fn=%u %s", fn,
               status == TOX_CONNECTION_NONE ? "offline" : "online");
    uint8_t pubkey[TOX_PUBLIC_KEY_SIZE];
    tox_friend_get_public_key(tox, fn, pubkey, NULL);
    vc args(VC_VECTOR);
    args.append(vc((int)fn));
    args.append(pubkey_to_vc(pubkey));
    args.append(vc(status == TOX_CONNECTION_NONE ? "offline" : "online"));
    p->event_cb("friend_connection_status", args, p->event_userdata);
}

static void
toxd_on_friend_name(Tox *tox, uint32_t fn, const uint8_t *name, size_t len, void *ud)
{
    ToxPlugin *p = (ToxPlugin *)ud;
    log_printf(p->log_file, "[cb] friend_name fn=%u name=%.*s", fn, (int)len, (const char *)name);
    uint8_t pubkey[TOX_PUBLIC_KEY_SIZE];
    tox_friend_get_public_key(tox, fn, pubkey, NULL);
    vc args(VC_VECTOR);
    args.append(vc((int)fn));
    args.append(pubkey_to_vc(pubkey));
    args.append(vc(VC_STRING, (const char *)name, (long)len));
    p->event_cb("friend_name", args, p->event_userdata);
}

static void
toxd_on_friend_status_message(Tox *tox, uint32_t fn, const uint8_t *msg,
                              size_t len, void *ud)
{
    ToxPlugin *p = (ToxPlugin *)ud;
    log_printf(p->log_file, "[cb] friend_status_message fn=%u msg=%.*s", fn, (int)len, (const char *)msg);
    uint8_t pubkey[TOX_PUBLIC_KEY_SIZE];
    tox_friend_get_public_key(tox, fn, pubkey, NULL);
    vc args(VC_VECTOR);
    args.append(vc((int)fn));
    args.append(pubkey_to_vc(pubkey));
    args.append(vc(VC_STRING, (const char *)msg, (long)len));
    p->event_cb("friend_status_message", args, p->event_userdata);
}

static void
toxd_on_friend_typing(Tox *tox, uint32_t fn, bool typing, void *ud)
{
    (void)tox;
    ToxPlugin *p = (ToxPlugin *)ud;
    log_printf(p->log_file, "[cb] friend_typing fn=%u typing=%d", fn, typing);
    uint8_t pubkey[TOX_PUBLIC_KEY_SIZE];
    tox_friend_get_public_key(tox, fn, pubkey, NULL);
    vc args(VC_VECTOR);
    args.append(vc((int)fn));
    args.append(pubkey_to_vc(pubkey));
    args.append(vc(typing ? "on" : "off"));
    p->event_cb("typing", args, p->event_userdata);
}

static void
toxd_on_friend_status(Tox *tox, uint32_t fn, Tox_User_Status status, void *ud)
{
    ToxPlugin *p = (ToxPlugin *)ud;
    const char *status_str = "none";
    if(status == TOX_USER_STATUS_AWAY)
        status_str = "away";
    else if(status == TOX_USER_STATUS_BUSY)
        status_str = "busy";
    log_printf(p->log_file, "[cb] friend_status fn=%u status=%s", fn, status_str);
    uint8_t pubkey[TOX_PUBLIC_KEY_SIZE];
    tox_friend_get_public_key(tox, fn, pubkey, NULL);
    vc args(VC_VECTOR);
    args.append(vc((int)fn));
    args.append(pubkey_to_vc(pubkey));
    args.append(vc(status_str));
    p->event_cb("friend_status", args, p->event_userdata);
}

// --- internal helpers ---

static void
save_tox_state(ToxPlugin *p)
{
    DwString save_path = DwString("%1/%2").arg(p->data_dir, "tox_save.tox");
    DwString tmp_path = DwString("%1/%2.new").arg(p->data_dir, "tox_save.tox");

    size_t sz = tox_get_savedata_size(p->tox);
    uint8_t *data = (uint8_t *)malloc(sz);
    tox_get_savedata(p->tox, data);

    FILE *f = fopen(tmp_path.c_str(), "wb");
    if(f)
    {
        fwrite(data, 1, sz, f);
        fclose(f);
        rename(tmp_path.c_str(), save_path.c_str());
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
load_or_create_tox(ToxPlugin *p)
{
    DwString save_path = DwString("%1/%2").arg(p->data_dir, "tox_save.tox");

    Tox_Err_Options_New new_err;
    struct Tox_Options &opts = *tox_options_new(&new_err);
    tox_options_default(&opts);
    tox_options_set_experimental_disable_dns(&opts, true);

    const char *home = getenv("HOME");
    DwString log_path;
    if(home)
        log_path = DwString("%1/tox.log").arg(home);
    else
        log_path = DwString("tox.log");
    p->log_file = fopen(log_path.c_str(), "a");
    if(p->log_file)
    {
        opts.log_callback = tox_write_log;
        opts.log_user_data = p->log_file;
    }

    uint8_t *savedata = NULL;
    size_t savedata_sz = 0;
    FILE *f = fopen(save_path.c_str(), "rb");
    if(f)
    {
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        rewind(f);
        if(sz > 0 && sz < 10 * 1024 * 1024)
        {
            savedata = (uint8_t *)malloc((size_t)sz);
            if(fread(savedata, 1, (size_t)sz, f) == (size_t)sz)
                savedata_sz = (size_t)sz;
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
    p->tox = tox_new(&opts, &err);
    if(savedata)
        free(savedata);
    if(!p->tox)
    {
        tox_options_default(&opts);
        tox_options_set_experimental_disable_dns(&opts, true);
        opts.savedata_type = TOX_SAVEDATA_TYPE_NONE;
        p->tox = tox_new(&opts, &err);
    }

    if(p->tox)
    {
        uint8_t address[TOX_ADDRESS_SIZE];
        tox_self_get_address(p->tox, address);
        char hex[TOX_ADDRESS_SIZE * 2 + 1];
        sodium_bin2hex(hex, sizeof(hex), address, TOX_ADDRESS_SIZE);
        fprintf(stderr, "toxd: %s %s\n", savedata ? "loaded" : "created", hex);

        if(!savedata)
            save_tox_state(p);
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
    tox_callback_friend_status(tox, toxd_on_friend_status);
}

// --- plugin API ---

ToxPlugin *
toxp_init(const char *data_dir, ToxpEventCB cb, void *userdata)
{
    ToxPlugin *p = new ToxPlugin;
    p->event_cb = cb;
    p->event_userdata = userdata;

    if(data_dir && data_dir[0])
    {
        p->data_dir = data_dir;
    }
    else
    {
        const char *env = getenv("DWYCO_TOX_DATA");
        if(env)
            p->data_dir = env;
        else
        {
            const char *home = getenv("HOME");
            if(home)
                p->data_dir = DwString("%1/%2").arg(home, ".config/dwyco/tox");
            else
                p->data_dir = DwString(".config/dwyco/tox");
        }
    }

    DwString lock_path = DwString("%1/%2").arg(p->data_dir, "toxd.lock");
    p->lock_fd = open(lock_path.c_str(), O_CREAT | O_RDWR, 0644);
    if(p->lock_fd < 0)
    {
        delete p;
        return NULL;
    }
    if(flock(p->lock_fd, LOCK_EX | LOCK_NB) < 0)
    {
        close(p->lock_fd);
        delete p;
        return NULL;
    }
    p->lock_path = lock_path;

    load_or_create_tox(p);
    if(!p->tox)
    {
        close(p->lock_fd);
        unlink(p->lock_path.c_str());
        delete p;
        return NULL;
    }

    register_callbacks(p->tox);
    return p;
}

void
toxp_shutdown(ToxPlugin *p)
{
    if(!p)
        return;
    save_tox_state(p);
    tox_kill(p->tox);
    if(p->lock_fd >= 0)
    {
        close(p->lock_fd);
        unlink(p->lock_path.c_str());
    }
    if(p->log_file)
        fclose(p->log_file);
    delete p;
}

void
toxp_save(ToxPlugin *p)
{
    save_tox_state(p);
}

void
toxp_iterate(ToxPlugin *p)
{
    tox_iterate(p->tox, p);
    if(!p->bootstrapped)
    {
        tox_bootstrap(p->tox);
        p->bootstrapped = 1;
    }
}

vc
toxp_get_address(ToxPlugin *p)
{
    uint8_t address[TOX_ADDRESS_SIZE];
    tox_self_get_address(p->tox, address);
    vc result(VC_MAP, "", 2);
    result.add_kv("address", vc(VC_BSTRING, (const char *)address, TOX_ADDRESS_SIZE));
    return result;
}

vc
toxp_get_self_pubkey(ToxPlugin *p)
{
    uint8_t pubkey[TOX_PUBLIC_KEY_SIZE];
    tox_self_get_public_key(p->tox, pubkey);
    uint8_t address[TOX_ADDRESS_SIZE];
    tox_self_get_address(p->tox, address);

    vc result(VC_MAP, "", 4);
    result.add_kv("pubkey", vc(VC_BSTRING, (const char *)pubkey, TOX_PUBLIC_KEY_SIZE));
    result.add_kv("address", vc(VC_BSTRING, (const char *)address, TOX_ADDRESS_SIZE));
    return result;
}

vc
toxp_get_name(ToxPlugin *p)
{
    size_t len = tox_self_get_name_size(p->tox);
    uint8_t *name = (uint8_t *)malloc(len + 1);
    tox_self_get_name(p->tox, name);
    name[len] = 0;
    vc result(VC_MAP, "", 2);
    result.add_kv("name", vc(VC_BSTRING, (const char *)name, (long)len));
    free(name);
    return result;
}

vc
toxp_get_status_message(ToxPlugin *p)
{
    size_t len = tox_self_get_status_message_size(p->tox);
    uint8_t *msg = (uint8_t *)malloc(len + 1);
    tox_self_get_status_message(p->tox, msg);
    msg[len] = 0;
    vc result(VC_MAP, "", 2);
    result.add_kv("status_message", vc(VC_BSTRING, (const char *)msg, (long)len));
    free(msg);
    return result;
}

int
toxp_set_name(ToxPlugin *p, const char *name, int len)
{
    Tox_Err_Set_Info err;
    if(!tox_self_set_name(p->tox, (const uint8_t *)name, (size_t)len, &err))
        return 0;
    return 1;
}

int
toxp_set_status_message(ToxPlugin *p, const char *msg, int len)
{
    Tox_Err_Set_Info err;
    if(!tox_self_set_status_message(p->tox, (const uint8_t *)msg, (size_t)len, &err))
        return 0;
    return 1;
}

void
toxp_set_user_status(ToxPlugin *p, const char *status)
{
    Tox_User_Status ts = TOX_USER_STATUS_NONE;
    if(strcmp(status, "away") == 0)
        ts = TOX_USER_STATUS_AWAY;
    else if(strcmp(status, "busy") == 0)
        ts = TOX_USER_STATUS_BUSY;
    tox_self_set_status(p->tox, ts);
}

vc
toxp_get_user_status(ToxPlugin *p)
{
    Tox_User_Status ts = tox_self_get_status(p->tox);
    const char *status_str = "none";
    if(ts == TOX_USER_STATUS_AWAY)
        status_str = "away";
    else if(ts == TOX_USER_STATUS_BUSY)
        status_str = "busy";
    vc result(VC_MAP, "", 2);
    result.add_kv("status", vc(status_str));
    return result;
}

int
toxp_friend_add(ToxPlugin *p, const vc &address, const vc &message, uint32_t *fn_out)
{
    if(address.len() != TOX_ADDRESS_SIZE || fn_out == NULL)
        return 0;
    uint8_t addr[TOX_ADDRESS_SIZE];
    memcpy(addr, (const char *)address, TOX_ADDRESS_SIZE);
    const uint8_t *msg = (const uint8_t *)(const char *)message;
    size_t mlen = (size_t)message.len();

    Tox_Err_Friend_Add err;
    uint32_t fn = tox_friend_add(p->tox, addr, msg, mlen, &err);
    if(err != TOX_ERR_FRIEND_ADD_OK)
        return 0;

    *fn_out = fn;
    save_tox_state(p);
    return 1;
}

int
toxp_friend_add_norequest(ToxPlugin *p, const vc &pubkey, uint32_t *fn_out)
{
    if(pubkey.len() != TOX_PUBLIC_KEY_SIZE || fn_out == NULL)
        return 0;
    Tox_Public_Key pk;
    memcpy(pk, (const char *)pubkey, TOX_PUBLIC_KEY_SIZE);

    Tox_Err_Friend_Add err;
    uint32_t fn = tox_friend_add_norequest(p->tox, pk, &err);
    if(err != TOX_ERR_FRIEND_ADD_OK)
        return 0;

    *fn_out = fn;
    save_tox_state(p);
    return 1;
}

int
toxp_friend_delete(ToxPlugin *p, uint32_t fn)
{
    Tox_Err_Friend_Delete err;
    tox_friend_delete(p->tox, fn, &err);
    if(err != TOX_ERR_FRIEND_DELETE_OK)
        return 0;
    save_tox_state(p);
    return 1;
}

int
toxp_message_send(ToxPlugin *p, uint32_t fn, const vc &text, int is_action,
                  uint32_t *mid_out, int *tox_err_out)
{
    Tox_Message_Type mtype = is_action ? TOX_MESSAGE_TYPE_ACTION : TOX_MESSAGE_TYPE_NORMAL;
    const uint8_t *msg = (const uint8_t *)(const char *)text;
    size_t mlen = (size_t)text.len();

    Tox_Err_Friend_Send_Message err;
    uint32_t mid = tox_friend_send_message(p->tox, fn, mtype, msg, mlen, &err);
    if(err != TOX_ERR_FRIEND_SEND_MESSAGE_OK)
    {
        if(tox_err_out)
            *tox_err_out = (int)err;
        return 0;
    }
    if(mid_out)
        *mid_out = mid;
    return 1;
}

int
toxp_typing_set(ToxPlugin *p, uint32_t fn, int typing)
{
    Tox_Err_Set_Typing err;
    if(!tox_self_set_typing(p->tox, fn, typing != 0, &err))
        return 0;
    return 1;
}

int
toxp_file_send(ToxPlugin *p, uint32_t fn, const vc &name, uint64_t size,
               uint32_t *fnum_out)
{
    const uint8_t *fname = (const uint8_t *)(const char *)name;
    size_t fnlen = (size_t)name.len();

    Tox_Err_File_Send err;
    uint32_t fnum = tox_file_send(p->tox, fn, TOX_FILE_KIND_DATA, size, NULL, fname, fnlen, &err);
    if(err != TOX_ERR_FILE_SEND_OK)
        return 0;
    if(fnum_out)
        *fnum_out = fnum;
    return 1;
}

int
toxp_file_send_data(ToxPlugin *p, uint32_t fn, uint32_t fnum, uint64_t pos,
                    const vc &data)
{
    Tox_Err_File_Send_Chunk err;
    if(data.is_nil() || data.len() == 0)
        tox_file_send_chunk(p->tox, fn, fnum, pos, NULL, 0, &err);
    else
    {
        const uint8_t *d = (const uint8_t *)(const char *)data;
        size_t dlen = (size_t)data.len();
        tox_file_send_chunk(p->tox, fn, fnum, pos, d, dlen, &err);
    }
    if(err != TOX_ERR_FILE_SEND_CHUNK_OK)
        return 0;
    return 1;
}

int
toxp_file_accept(ToxPlugin *p, uint32_t fn, uint32_t fnum)
{
    Tox_Err_File_Control err;
    tox_file_control(p->tox, fn, fnum, TOX_FILE_CONTROL_RESUME, &err);
    if(err != TOX_ERR_FILE_CONTROL_OK)
        return 0;
    return 1;
}

vc
toxp_friend_list(ToxPlugin *p)
{
    uint32_t n = tox_self_get_friend_list_size(p->tox);
    std::vector<uint32_t> friend_list(n);
    tox_self_get_friend_list(p->tox, friend_list.data());
    vc friends(VC_VECTOR);
    for(uint32_t i = 0; i < n; ++i)
    {
        uint32_t fn = friend_list[i];
        uint8_t pubkey[TOX_PUBLIC_KEY_SIZE];
        if(tox_friend_get_public_key(p->tox, fn, pubkey, NULL))
        {
            vc entry(VC_MAP, "", 8);
            entry.add_kv("friend_number", vc((int)fn));
            entry.add_kv("pubkey", vc(VC_BSTRING, (const char *)pubkey, TOX_PUBLIC_KEY_SIZE));

            size_t name_len = tox_friend_get_name_size(p->tox, fn, NULL);
            if(name_len > 0 && name_len <= TOX_MAX_NAME_LENGTH)
            {
                uint8_t name[TOX_MAX_NAME_LENGTH];
                tox_friend_get_name(p->tox, fn, name, NULL);
                entry.add_kv("name", vc(VC_BSTRING, (const char *)name, (long)name_len));
            }
            else
                entry.add_kv("name", vc(""));

            Tox_Connection conn = tox_friend_get_connection_status(p->tox, fn, NULL);
            entry.add_kv("status", vc(conn == TOX_CONNECTION_NONE ? "offline" : "online"));

            Tox_User_Status us = tox_friend_get_status(p->tox, fn, NULL);
            entry.add_kv("user_status", vc(us == TOX_USER_STATUS_AWAY ? "away" : us == TOX_USER_STATUS_BUSY ? "busy" : "none"));

            friends.append(entry);
        }
    }
    vc result(VC_MAP, "", 2);
    result.add_kv("friends", friends);
    return result;
}

// --- old-style main kept for standalone test builds ---
#if defined(TOXD_STANDALONE)

struct StandaloneCtx {
    ToxPlugin *plugin;
};

static void
standalone_on_event(const char *type, const vc &args, void *userdata)
{
    StandaloneCtx *ctx = (StandaloneCtx *)userdata;
    log_printf(ctx->plugin->log_file, "[event] %s args=%d", type, args.num_elems());
    for(int i = 0; i < args.num_elems(); ++i)
    {
        vc v = args[i];
        DwString s;
        s = v.peek_str();
        log_printf(ctx->plugin->log_file, "  [%d] %s", i, s.c_str());
    }
}

void
oopanic(const char *)
{
    ::abort();
}

int
main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    StandaloneCtx ctx = {};
    ToxPlugin *p = toxp_init(NULL, standalone_on_event, &ctx);
    if(!p)
        return 1;
    ctx.plugin = p;
    log_printf(p->log_file, "START!");
    while(1)
    {
        toxp_iterate(p);
    }
    toxp_shutdown(p);
    return 0;
}
#endif

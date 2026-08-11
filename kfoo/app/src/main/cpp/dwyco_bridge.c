/*
 * C JNI bridge between Kotlin (com.dwyco.kfoo.DwycoNative) and the
 * prebuilt libdwyco_jni.so core library (self-contained C API, dlli.h).
 *
 * Compiles as plain C (no C++ runtime). It links the prebuilt core
 * and marshals the core's callbacks onto the Kotlin event sink.
 */
#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dlli.h"

#define LOG_TAG "kfoo"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static JavaVM *g_vm;
static jobject g_sink;
static jclass g_sink_class;
static jmethodID g_m_login;
static jmethodID g_m_sys;
static jmethodID g_m_chat;
static jmethodID g_m_emergency;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static void
hex_encode(const char *in, int len, char *out)
{
    static const char hex[] = "0123456789abcdef";
    int i;
    for(i = 0; i < len; ++i)
    {
        unsigned char c = (unsigned char)in[i];
        out[i*2] = hex[c >> 4];
        out[i*2+1] = hex[c & 0xf];
    }
    out[len*2] = 0;
}

static int
hex_val(char c)
{
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'a' && c <= 'f') return c - 'a' + 10;
    if(c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int
hex_decode(const char *in, char *out, int maxlen)
{
    int n = 0;
    while(in[0] && in[1] && n < maxlen)
    {
        int hi = hex_val(in[0]);
        int lo = hex_val(in[1]);
        if(hi < 0 || lo < 0)
            break;
        out[n++] = (char)((hi << 4) | lo);
        in += 2;
    }
    return n;
}

static JNIEnv *
get_env(int *detach)
{
    JNIEnv *env = 0;
    *detach = 0;
    if((*g_vm)->GetEnv(g_vm, (void **)&env, JNI_VERSION_1_6) == JNI_EDETACHED)
    {
        if((*g_vm)->AttachCurrentThread(g_vm, &env, 0) != 0)
            return 0;
        *detach = 1;
    }
    return env;
}

static jstring
js_bytes(JNIEnv *env, const char *data, int len, int as_hex)
{
    char *buf;
    jstring ret;
    if(!data || len <= 0)
        return (*env)->NewStringUTF(env, "");
    buf = (char *)malloc(len*2+1);
    if(!buf)
        return (*env)->NewStringUTF(env, "");
    if(as_hex)
        hex_encode(data, len, buf);
    else
    {
        memcpy(buf, data, len);
        buf[len] = 0;
    }
    ret = (*env)->NewStringUTF(env, buf);
    free(buf);
    return ret;
}

static void
dispatch_event(JNIEnv *env, jmethodID method, int cmd, int id,
               const char *uid, int len_uid,
               const char *name, int len_name,
               int type, const char *val, int len_val,
               int qid, int extra)
{
    jstring juid = js_bytes(env, uid, len_uid, 1);
    jstring jname = js_bytes(env, name, len_name, 0);
    jstring jval = (type == DWYCO_TYPE_STRING) ? js_bytes(env, val, len_val, 0)
                                               : (*env)->NewStringUTF(env, "");
    (*env)->CallVoidMethod(env, g_sink, method, cmd, id, juid, jname, type, jval, qid, extra);
    (*env)->DeleteLocalRef(env, juid);
    (*env)->DeleteLocalRef(env, jname);
    (*env)->DeleteLocalRef(env, jval);
}

static void
DWYCOCALLCONV
sys_event_cb(int cmd, int id, const char *uid, int len_uid,
             const char *name, int len_name, int type,
             const char *val, int len_val, int qid, int extra)
{
    JNIEnv *env;
    int detach;
    if(!g_sink || !g_m_sys)
        return;
    pthread_mutex_lock(&g_lock);
    env = get_env(&detach);
    if(env)
        dispatch_event(env, g_m_sys, cmd, id, uid, len_uid, name, len_name, type, val, len_val, qid, extra);
    if(detach)
        (*g_vm)->DetachCurrentThread(g_vm);
    pthread_mutex_unlock(&g_lock);
}

static void
DWYCOCALLCONV
chat_ctx_cb(int cmd, int id, const char *uid, int len_uid,
            const char *name, int len_name, int type,
            const char *val, int len_val, int qid, int extra)
{
    JNIEnv *env;
    int detach;
    if(!g_sink || !g_m_chat)
        return;
    pthread_mutex_lock(&g_lock);
    env = get_env(&detach);
    if(env)
        dispatch_event(env, g_m_chat, cmd, id, uid, len_uid, name, len_name, type, val, len_val, qid, extra);
    if(detach)
        (*g_vm)->DetachCurrentThread(g_vm);
    pthread_mutex_unlock(&g_lock);
}

static void
DWYCOCALLCONV
login_result_cb(const char *str, int what)
{
    JNIEnv *env;
    int detach;
    if(!g_sink || !g_m_login)
        return;
    pthread_mutex_lock(&g_lock);
    env = get_env(&detach);
    if(env)
    {
        jstring js = js_bytes(env, str, str ? (int)strlen(str) : 0, 0);
        (*env)->CallVoidMethod(env, g_sink, g_m_login, js, what);
        (*env)->DeleteLocalRef(env, js);
    }
    if(detach)
        (*g_vm)->DetachCurrentThread(g_vm);
    pthread_mutex_unlock(&g_lock);
}

static void
DWYCOCALLCONV
emergency_cb(int what, int must_exit, const char *msg)
{
    JNIEnv *env;
    int detach;
    if(!g_sink || !g_m_emergency)
        return;
    pthread_mutex_lock(&g_lock);
    env = get_env(&detach);
    if(env)
    {
        jstring js = js_bytes(env, msg, msg ? (int)strlen(msg) : 0, 0);
        (*env)->CallVoidMethod(env, g_sink, g_m_emergency, what, must_exit, js);
        (*env)->DeleteLocalRef(env, js);
    }
    if(detach)
        (*g_vm)->DetachCurrentThread(g_vm);
    pthread_mutex_unlock(&g_lock);
}

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved)
{
    g_vm = vm;
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL
Java_com_dwyco_kfoo_DwycoNative_nativeSetEventSink(JNIEnv *env, jobject thiz, jobject sink)
{
    jclass cls;
    if(g_sink)
    {
        (*env)->DeleteGlobalRef(env, g_sink);
        g_sink = 0;
    }
    if(g_sink_class)
    {
        (*env)->DeleteGlobalRef(env, g_sink_class);
        g_sink_class = 0;
    }
    if(!sink)
        return;
    cls = (*env)->GetObjectClass(env, sink);
    g_sink_class = (*env)->NewGlobalRef(env, cls);
    g_sink = (*env)->NewGlobalRef(env, sink);
    g_m_login = (*env)->GetMethodID(env, g_sink_class, "onLogin", "(Ljava/lang/String;I)V");
    g_m_sys = (*env)->GetMethodID(env, g_sink_class, "onSystemEvent", "(IILjava/lang/String;Ljava/lang/String;ILjava/lang/String;II)V");
    g_m_chat = (*env)->GetMethodID(env, g_sink_class, "onChatEvent", "(IILjava/lang/String;Ljava/lang/String;ILjava/lang/String;II)V");
    g_m_emergency = (*env)->GetMethodID(env, g_sink_class, "onEmergency", "(IILjava/lang/String;)V");
    (*env)->DeleteLocalRef(env, cls);
}

JNIEXPORT jint JNICALL
Java_com_dwyco_kfoo_DwycoNative_nativeInit(JNIEnv *env, jobject thiz,
                                           jstring sysPfx, jstring userPfx, jstring tmpPfx)
{
    const char *sp = 0, *up = 0, *tp = 0;
    if(sysPfx) sp = (*env)->GetStringUTFChars(env, sysPfx, 0);
    if(userPfx) up = (*env)->GetStringUTFChars(env, userPfx, 0);
    if(tmpPfx) tp = (*env)->GetStringUTFChars(env, tmpPfx, 0);

    dwyco_set_fn_prefixes(sp, up, tp);
    dwyco_trace_init();
    dwyco_set_login_result_callback(login_result_cb);
    dwyco_set_chat_ctx_callback(chat_ctx_cb);
    dwyco_set_system_event_callback(sys_event_cb);
    dwyco_set_emergency_callback(emergency_cb);

    if(!dwyco_init())
    {
        LOGE("dwyco_init failed");
        if(sp) (*env)->ReleaseStringUTFChars(env, sysPfx, sp);
        if(up) (*env)->ReleaseStringUTFChars(env, userPfx, up);
        if(tp) (*env)->ReleaseStringUTFChars(env, tmpPfx, tp);
        return 0;
    }
    dwyco_set_disposition("foreground", 10);
    dwyco_set_setting("zap/always_server", "0");
    dwyco_set_setting("call_acceptance/auto_accept", "0");
    dwyco_set_setting("net/listen", "1");
    dwyco_set_setting("net/app_id", "kfoo");
    dwyco_set_setting("net/broadcast_port", "48903");

    if(sp) (*env)->ReleaseStringUTFChars(env, sysPfx, sp);
    if(up) (*env)->ReleaseStringUTFChars(env, userPfx, up);
    if(tp) (*env)->ReleaseStringUTFChars(env, tmpPfx, tp);
    return 1;
}

JNIEXPORT jint JNICALL
Java_com_dwyco_kfoo_DwycoNative_nativeIsNewAccount(JNIEnv *env, jobject thiz)
{
    return dwyco_get_create_new_account();
}

JNIEXPORT jint JNICALL
Java_com_dwyco_kfoo_DwycoNative_nativeBootstrap(JNIEnv *env, jobject thiz,
                                                jstring handle, jstring email)
{
    const char *h = 0, *e = 0;
    if(handle) h = (*env)->GetStringUTFChars(env, handle, 0);
    if(email) e = (*env)->GetStringUTFChars(env, email, 0);
    dwyco_create_bootstrap_profile(h, h ? (int)strlen(h) : 0,
                                   "", 0,
                                   "mobile user", 11,
                                   e, e ? (int)strlen(e) : 0);
    dwyco_set_local_auth(1);
    dwyco_finish_startup();
    if(h) (*env)->ReleaseStringUTFChars(env, handle, h);
    if(e) (*env)->ReleaseStringUTFChars(env, email, e);
    return 1;
}

JNIEXPORT jint JNICALL
Java_com_dwyco_kfoo_DwycoNative_nativeStartup(JNIEnv *env, jobject thiz)
{
    dwyco_set_local_auth(1);
    dwyco_finish_startup();
    return 1;
}

JNIEXPORT jint JNICALL
Java_com_dwyco_kfoo_DwycoNative_nativeServiceChannels(JNIEnv *env, jobject thiz)
{
    int spin = 0;
    dwyco_service_channels(&spin);
    return spin;
}

JNIEXPORT jint JNICALL
Java_com_dwyco_kfoo_DwycoNative_nativeSendText(JNIEnv *env, jobject thiz,
                                               jstring uidHex, jstring text)
{
    const char *huid = (*env)->GetStringUTFChars(env, uidHex, 0);
    const char *txt = (*env)->GetStringUTFChars(env, text, 0);
    char bin[64];
    int blen = hex_decode(huid, bin, sizeof bin);
    int compid = dwyco_make_zap_composition(0);
    if(compid == 0)
    {
        (*env)->ReleaseStringUTFChars(env, uidHex, huid);
        (*env)->ReleaseStringUTFChars(env, text, txt);
        return 0;
    }
    if(!dwyco_zap_send5(compid, bin, blen, txt, (int)strlen(txt), 0, 1, 0, 0))
    {
        dwyco_delete_zap_composition(compid);
        compid = 0;
    }
    (*env)->ReleaseStringUTFChars(env, uidHex, huid);
    (*env)->ReleaseStringUTFChars(env, text, txt);
    return compid;
}

JNIEXPORT jobjectArray JNICALL
Java_com_dwyco_kfoo_DwycoNative_nativeGetConversations(JNIEnv *env, jobject thiz)
{
    DWYCO_USER_LIST l;
    int n;
    jobjectArray ret;
    jclass strcls = (*env)->FindClass(env, "java/lang/String");
    if(!dwyco_get_user_list2(&l, &n))
        return (*env)->NewObjectArray(env, 0, strcls, 0);
    ret = (*env)->NewObjectArray(env, n, strcls, 0);
    if(!ret)
    {
        dwyco_list_release(l);
        return 0;
    }
    {
        int i;
        for(i = 0; i < n; ++i)
        {
            const char *out;
            int len, type;
            if(dwyco_list_get(l, i, DWYCO_NO_COLUMN, &out, &len, &type))
            {
                jstring js = js_bytes(env, out, len, 1);
                (*env)->SetObjectArrayElement(env, ret, i, js);
                (*env)->DeleteLocalRef(env, js);
            }
        }
    }
    dwyco_list_release(l);
    return ret;
}

JNIEXPORT jobjectArray JNICALL
Java_com_dwyco_kfoo_DwycoNative_nativeGetMessages(JNIEnv *env, jobject thiz, jstring uidHex)
{
    const char *huid = (*env)->GetStringUTFChars(env, uidHex, 0);
    char bin[64];
    int blen = hex_decode(huid, bin, sizeof bin);
    DWYCO_MSG_IDX mi = 0;
    int rows = 0;
    jobjectArray ret;
    jclass strcls = (*env)->FindClass(env, "java/lang/String");
    int i;
    (*env)->ReleaseStringUTFChars(env, uidHex, huid);
    if(blen <= 0)
        return (*env)->NewObjectArray(env, 0, strcls, 0);
    if(!dwyco_get_message_index(&mi, bin, blen))
        return (*env)->NewObjectArray(env, 0, strcls, 0);
    dwyco_list_numelems(mi, &rows, 0);
    ret = (*env)->NewObjectArray(env, rows, strcls, 0);
    if(!ret)
    {
        dwyco_list_release(mi);
        return 0;
    }
    for(i = 0; i < rows; ++i)
    {
        char midbuf[256];
        char textbuf[2048];
        int sent = 0;
        int midlen = 0;
        const char *out;
        int len, type;
        midbuf[0] = 0;
        textbuf[0] = 0;
        if(dwyco_list_get(mi, i, DWYCO_MSG_IDX_MID, &out, &len, &type) &&
           type != DWYCO_TYPE_NIL && len < (int)sizeof midbuf)
        {
            memcpy(midbuf, out, len);
            midbuf[len] = 0;
            midlen = len;
        }
        if(dwyco_list_get(mi, i, DWYCO_MSG_IDX_IS_SENT, &out, &len, &type) &&
           type != DWYCO_TYPE_NIL)
            sent = 1;
        if(midlen > 0)
        {
            DWYCO_SAVED_MSG_LIST sm;
            int st = dwyco_get_saved_message3(&sm, midbuf);
            if(st == DWYCO_GSM_SUCCESS)
            {
                DWYCO_LIST ba = dwyco_get_body_array(sm);
                int brows, bcols;
                dwyco_list_numelems(ba, &brows, 0);
                if(brows > 1)
                {
                    DWYCO_LIST bt = dwyco_get_body_text(ba);
                    const char *to;
                    int tl, tt;
                    if(bt && dwyco_list_get(bt, 0, DWYCO_NO_COLUMN, &to, &tl, &tt) &&
                       tl < (int)sizeof textbuf)
                    {
                        memcpy(textbuf, to, tl);
                        textbuf[tl] = 0;
                    }
                    if(bt)
                        dwyco_list_release(bt);
                }
                else
                {
                    const char *to;
                    int tl, tt;
                    if(dwyco_list_get(ba, 0, DWYCO_QM_BODY_NEW_TEXT2, &to, &tl, &tt) &&
                       tt != DWYCO_TYPE_NIL && tl < (int)sizeof textbuf)
                    {
                        memcpy(textbuf, to, tl);
                        textbuf[tl] = 0;
                    }
                    else if(dwyco_list_get(ba, 0, DWYCO_QM_BODY_TEXT2_obsolete, &to, &tl, &tt) &&
                            tt != DWYCO_TYPE_NIL && tl < (int)sizeof textbuf)
                    {
                        memcpy(textbuf, to, tl);
                        textbuf[tl] = 0;
                    }
                }
                dwyco_list_release(ba);
                dwyco_list_release(sm);
            }
            else
                snprintf(textbuf, sizeof textbuf, "(unknown)");
        }
        {
            char outbuf[2400];
            jstring js;
            snprintf(outbuf, sizeof outbuf, "%s\t%d\t%s", midbuf, sent, textbuf);
            js = (*env)->NewStringUTF(env, outbuf);
            (*env)->SetObjectArrayElement(env, ret, i, js);
            (*env)->DeleteLocalRef(env, js);
        }
    }
    dwyco_list_release(mi);
    return ret;
}

JNIEXPORT jstring JNICALL
Java_com_dwyco_kfoo_DwycoNative_nativeGetMyUid(JNIEnv *env, jobject thiz)
{
    const char *uid = 0;
    int len = 0;
    dwyco_get_my_uid(&uid, &len);
    return js_bytes(env, uid, len, 1);
}

JNIEXPORT jint JNICALL
Java_com_dwyco_kfoo_DwycoNative_nativeExit(JNIEnv *env, jobject thiz)
{
    return dwyco_exit();
}

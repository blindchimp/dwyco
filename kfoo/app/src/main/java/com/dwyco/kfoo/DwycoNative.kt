package com.dwyco.kfoo

interface NativeEvents {
    fun onLogin(message: String, what: Int)
    fun onSystemEvent(cmd: Int, id: Int, uidHex: String, name: String, type: Int, value: String, qid: Int, extra: Int)
    fun onChatEvent(cmd: Int, id: Int, uidHex: String, name: String, type: Int, value: String, qid: Int, extra: Int)
    fun onEmergency(what: Int, mustExit: Int, message: String)
}

object DwycoEvents {
    const val USER_STATUS_CHANGE = 1
    const val USER_ADD = 2
    const val SERVER_CONNECTING = 4
    const val SERVER_CONNECTION_SUCCESSFUL = 5
    const val SERVER_DISCONNECT = 6
    const val SERVER_LOGIN = 7
    const val SERVER_LOGIN_FAILED = 8
    const val USER_MSG_RECEIVED = 9
    const val USER_UID_RESOLVED = 10
    const val USER_PROFILE_INVALIDATE = 11
    const val USER_MSG_IDX_UPDATED = 12
    const val MSG_SEND_START = 14
    const val MSG_SEND_FAIL = 15
    const val MSG_SEND_SUCCESS = 16
    const val MSG_DOWNLOAD_OK = 24
}

object DwycoNative {
    init {
        System.loadLibrary("dwyco_kotlin_jni")
    }

    external fun nativeSetEventSink(sink: NativeEvents)
    external fun nativeInit(sysPfx: String?, userPfx: String, tmpPfx: String): Int
    external fun nativeIsNewAccount(): Int
    external fun nativeBootstrap(handle: String, email: String): Int
    external fun nativeStartup(): Int
    external fun nativeServiceChannels(): Int
    external fun nativeSendText(uidHex: String, text: String): Int
    external fun nativeGetConversations(): Array<String>
    external fun nativeGetMessages(uidHex: String): Array<String>
    external fun nativeGetMyUid(): String
    external fun nativeExit(): Int
}

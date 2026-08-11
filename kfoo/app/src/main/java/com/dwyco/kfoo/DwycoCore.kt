package com.dwyco.kfoo

import android.content.Context
import android.os.Handler
import android.os.Looper
import java.io.File

object DwycoCore : NativeEvents {

    interface CoreListener {
        fun onLogin(message: String, success: Boolean)
        fun onNewMessage(uidHex: String)
        fun onResolvedName(uidHex: String, name: String)
        fun onEmergency(what: Int, mustExit: Boolean, message: String)
    }

    data class DwycoMessage(val mid: String, val isSent: Boolean, val text: String)

    @Volatile private var listener: CoreListener? = null
    @Volatile private var running = false
    private val mainHandler = Handler(Looper.getMainLooper())

    var initialized = false
        private set
    var newAccount = false
        private set
    var myUid: String = ""
        private set

    fun setListener(l: CoreListener?) {
        listener = l
    }

    fun init(context: Context): Boolean {
        if (initialized) return true
        val userDir = File(context.filesDir, "dwyco").absolutePath
        val tmpDir = File(context.cacheDir, "dwyco/tmp").absolutePath
        DwycoNative.nativeSetEventSink(this)
        val ok = DwycoNative.nativeInit(null, userDir, tmpDir) != 0
        if (!ok) return false
        initialized = true
        newAccount = DwycoNative.nativeIsNewAccount() != 0
        myUid = DwycoNative.nativeGetMyUid()
        running = true
        Thread({
            serviceLoop()
        }, "kfoo-service").apply {
            isDaemon = true
            start()
        }
        return true
    }

    private fun serviceLoop() {
        while (running) {
            val spin = DwycoNative.nativeServiceChannels()
            try {
                Thread.sleep(if (spin != 0) 1L else 100L)
            } catch (e: InterruptedException) {
                break
            }
        }
    }

    fun startup() {
        DwycoNative.nativeStartup()
    }

    fun bootstrap(handle: String, email: String): Int =
        DwycoNative.nativeBootstrap(handle, email)

    fun sendText(uidHex: String, text: String): Int =
        DwycoNative.nativeSendText(uidHex, text)

    fun loadConversations(): List<String> =
        DwycoNative.nativeGetConversations().toList()

    fun loadMessages(uidHex: String): List<DwycoMessage> =
        DwycoNative.nativeGetMessages(uidHex).mapNotNull { line ->
            val parts = line.split('\t', limit = 3)
            if (parts.size == 3)
                DwycoMessage(mid = parts[0], isSent = parts[1] == "1", text = parts[2])
            else null
        }

    fun exit() {
        running = false
        DwycoNative.nativeExit()
    }

    override fun onLogin(message: String, what: Int) {
        mainHandler.post { listener?.onLogin(message, what > 0) }
    }

    override fun onSystemEvent(cmd: Int, id: Int, uidHex: String, name: String, type: Int, value: String, qid: Int, extra: Int) {
        mainHandler.post {
            when (cmd) {
                DwycoEvents.USER_MSG_RECEIVED -> listener?.onNewMessage(uidHex)
                DwycoEvents.USER_UID_RESOLVED -> listener?.onResolvedName(uidHex, name)
            }
        }
    }

    override fun onChatEvent(cmd: Int, id: Int, uidHex: String, name: String, type: Int, value: String, qid: Int, extra: Int) {
        // chat room events, unused in this text-messaging scaffold
    }

    override fun onEmergency(what: Int, mustExit: Int, message: String) {
        mainHandler.post { listener?.onEmergency(what, mustExit != 0, message) }
    }
}

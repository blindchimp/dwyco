package com.dwyco.kfoo

import android.app.Activity
import android.os.Bundle
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.EditText
import android.widget.ListView
import android.widget.TextView
import android.widget.Toast

class ChatActivity : Activity(), DwycoCore.CoreListener {

    companion object {
        const val EXTRA_UID = "uid"
    }

    private lateinit var peerLabel: TextView
    private lateinit var messageList: ListView
    private lateinit var messageInput: EditText
    private lateinit var sendBtn: Button

    private var peerUid: String = ""
    private val messages = mutableListOf<String>()
    private lateinit var adapter: ArrayAdapter<String>

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_chat)

        peerUid = intent.getStringExtra(EXTRA_UID) ?: ""
        peerLabel = findViewById(R.id.peerLabel)
        messageList = findViewById(R.id.messageList)
        messageInput = findViewById(R.id.messageInput)
        sendBtn = findViewById(R.id.sendBtn)

        peerLabel.text = getString(R.string.conversations_title) + ": $peerUid"

        adapter = ArrayAdapter(this, android.R.layout.simple_list_item_1, messages)
        messageList.adapter = adapter
        messageList.transcriptMode = ListView.TRANSCRIPT_MODE_ALWAYS_SCROLL

        sendBtn.setOnClickListener { sendMessage() }

        DwycoCore.setListener(this)
        refreshMessages()
    }

    override fun onResume() {
        super.onResume()
        DwycoCore.setListener(this)
        refreshMessages()
    }

    private fun sendMessage() {
        val text = messageInput.text.toString().trim()
        if (text.isEmpty())
            return
        val compid = DwycoCore.sendText(peerUid, text)
        messageInput.text.clear()
        if (compid == 0)
            Toast.makeText(this, "Send failed", Toast.LENGTH_SHORT).show()
        refreshMessages()
    }

    private fun refreshMessages() {
        val msgs = DwycoCore.loadMessages(peerUid)
        messages.clear()
        messages.addAll(msgs.map { (if (it.isSent) "You: " else "Them: ") + it.text })
        adapter.notifyDataSetChanged()
    }

    override fun onNewMessage(uidHex: String) {
        if (uidHex == peerUid)
            refreshMessages()
    }

    override fun onLogin(message: String, success: Boolean) = Unit

    override fun onResolvedName(uidHex: String, name: String) = Unit

    override fun onEmergency(what: Int, mustExit: Boolean, message: String) {
        Toast.makeText(this, "Emergency: $message", Toast.LENGTH_LONG).show()
        if (mustExit)
            finishAffinity()
    }
}

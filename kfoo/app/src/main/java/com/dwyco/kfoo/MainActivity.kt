package com.dwyco.kfoo

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.view.View
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.ListView
import android.widget.TextView
import android.widget.Toast

class MainActivity : Activity(), DwycoCore.CoreListener {

    private lateinit var statusText: TextView
    private lateinit var bootstrapForm: LinearLayout
    private lateinit var handleInput: EditText
    private lateinit var emailInput: EditText
    private lateinit var bootstrapBtn: Button
    private lateinit var conversationList: ListView

    private val conversations = mutableListOf<String>()
    private lateinit var adapter: ArrayAdapter<String>
    private var chatServerSwitched = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        statusText = findViewById(R.id.statusText)
        bootstrapForm = findViewById(R.id.bootstrapForm)
        handleInput = findViewById(R.id.handleInput)
        emailInput = findViewById(R.id.emailInput)
        bootstrapBtn = findViewById(R.id.bootstrapBtn)
        conversationList = findViewById(R.id.conversationList)

        adapter = ArrayAdapter(this, android.R.layout.simple_list_item_1, conversations)
        conversationList.adapter = adapter
        conversationList.setOnItemClickListener { _, _, position, _ ->
            val uid = conversations[position]
            startActivity(Intent(this, ChatActivity::class.java).putExtra(ChatActivity.EXTRA_UID, uid))
        }
        bootstrapBtn.setOnClickListener { doBootstrap() }
        handleInput.setText("Ktest")

        DwycoCore.setListener(this)
        if (!DwycoCore.init(this)) {
            statusText.text = getString(R.string.status_init)
            Toast.makeText(this, "Native init failed", Toast.LENGTH_LONG).show()
            return
        }
        if (DwycoCore.newAccount) {
            bootstrapForm.visibility = View.VISIBLE
            statusText.text = getString(R.string.new_account_title)
        } else {
            statusText.text = getString(R.string.status_init)
            DwycoCore.startup()
            refreshConversations()
        }
    }

    override fun onResume() {
        super.onResume()
        DwycoCore.setListener(this)
        if (DwycoCore.initialized && !DwycoCore.newAccount)
            refreshConversations()
    }

    private fun doBootstrap() {
        val handle = handleInput.text.toString().trim()
        val email = emailInput.text.toString().trim()
        if (handle.isEmpty()) {
            Toast.makeText(this, "Enter a handle", Toast.LENGTH_SHORT).show()
            return
        }
        statusText.text = getString(R.string.status_creating)
        bootstrapBtn.isEnabled = false
        DwycoCore.bootstrap(handle, email)
    }

    private fun refreshConversations() {
        val uids = DwycoCore.loadConversations()
        conversations.clear()
        conversations.addAll(uids)
        adapter.notifyDataSetChanged()
        for (uid in uids)
            DwycoCore.fetchPendingMessages(uid)
        statusText.text = if (uids.isEmpty())
            getString(R.string.no_conversations)
        else
            getString(R.string.my_uid, DwycoCore.myUid)
    }

    override fun onLogin(message: String, success: Boolean) {
        statusText.text = message
        bootstrapBtn.isEnabled = true
        if (success) {
            bootstrapForm.visibility = View.GONE
            refreshConversations()
            if (!chatServerSwitched) {
                chatServerSwitched = true
                DwycoCore.switchToChatServer(0)
            }
        }
    }

    override fun onNewMessage(uidHex: String) {
        if (DwycoCore.initialized && !DwycoCore.newAccount)
            refreshConversations()
    }

    override fun onResolvedName(uidHex: String, name: String) {
        // names can decorate conversations later; uid is used for now
    }

    override fun onEmergency(what: Int, mustExit: Boolean, message: String) {
        Toast.makeText(this, "Emergency: $message", Toast.LENGTH_LONG).show()
        if (mustExit)
            finishAffinity()
    }
}

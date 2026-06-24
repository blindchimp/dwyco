import QtQuick
import dwyco

Rectangle {
    property string friendUid: ""

    property string _friendStatus: "offline"
    property string _friendUserStatus: "none"
    property bool _inFriendList: false

    radius: width / 2
    color: {
        if (!core.tox_enabled) return "purple"
        if (!_inFriendList) return "purple"
        if (_friendStatus === "offline") return "gray"
        if (_friendUserStatus === "busy") return "red"
        if (_friendUserStatus === "away") return "orange"
        return "limegreen"
    }
    visible: false
    z: 3

    Text {
        anchors.centerIn: parent
        text: "T"
        color: "white"
        font.bold: true
        font.pixelSize: parent.height * 0.6
    }

    onFriendUidChanged: lookupFriend()

    function lookupFriend() {
        if (!core.is_tox_uid(friendUid)) {
            visible = false
            return
        }
        visible = true
        for (var i = 0; i < ToxFriendModel.count; i++) {
            var f = ToxFriendModel.get(i)
            if (f.pubkey.substring(0, 20) === friendUid) {
                _inFriendList = true
                _friendStatus = f.status
                _friendUserStatus = f.user_status
                return
            }
        }
        _inFriendList = false
    }

    Component.onCompleted: lookupFriend()

    Connections {
        target: core
        function onTox_friend_status_changed(pseudo_uid, status) {
            if (pseudo_uid === friendUid) _friendStatus = status
        }
        function onTox_friend_user_status_changed(pseudo_uid, user_status) {
            if (pseudo_uid === friendUid) _friendUserStatus = user_status
        }
        function onTox_enabledChanged() {
            lookupFriend()
        }
    }

    Connections {
        target: ToxFriendModel
        function onCountChanged() {
            lookupFriend()
        }
    }
}

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

MessageDialog {
    signal noClicked
    signal yesClicked

//    title: "Bulk delete?"
//    //icon: StandardIcon.Question
//    text: "Delete ALL messages from selected users?"
//    informativeText: "This REMOVES FAVORITE messages too."
    buttons: MessageDialog.Yes | MessageDialog.No
    //detailedText: ""
    onButtonClicked: (button, role)=> {
                         switch(button) {
                             case MessageDialog.Yes: {
                                 yesClicked()
                                 break
                             }
                             case MessageDialog.No: {
                                 noClicked()
                                 break
                             }
                         }
                     }

//                onYesClicked: {
//                    model.delete_all_selected()
//                    multiselect_mode = false
//                    close()
//                }
//                onNoClicked: {
//                    close()
//                }
}

import QtCore
import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11

ApplicationWindow {
    id: window
    width: 800
    height: 500
    visible: true

    SystemPalette {
        id: activePalette;
        colorGroup: SystemPalette.Active
    }

    GridLayout {
        id: grid
        columns: 3
        width: parent.width - 5
        TextField {
            id: "videoUrl"
            placeholderText: "Video URL";
            Layout.columnSpan: 2;
            Layout.fillWidth: true
        }
        RowLayout {
            Button {
                text: "Load formats";
                onClicked: mf.loadFormats(username.text, videoUrl.text, cookiesFromBrowser.text, userAgent.text);
                Layout.fillWidth: false
            }
            Button {
                text: "Settings";
                onClicked: settingsDialog.open();
                Layout.fillWidth: false
            }
        }
        Button {
            id: start
            text: "Start";
            onClicked: mf.start(username.text, sublang.text, cookiesFromBrowser.text, userAgent.text);
            Layout.columnSpan: 3;
            Layout.fillWidth: true
        }
        ScrollView {
            implicitHeight: window.height - videoUrl.height - start.height - 20
            Layout.columnSpan: 3
            Layout.fillWidth: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn

            contentItem: ListView {
                id: listView
                model: mf.formatsModel
                spacing: 2
                highlightMoveDuration: 100
                highlightMoveVelocity: -1
                leftMargin: 2
                topMargin: 2
                bottomMargin: 2
                rightMargin: 2
                boundsMovement: Flickable.StopAtBounds
                clip: true

                property int width0: 20
                property int width1: listView.width / 22 * 5
                property int width2: listView.width / 22 * 3
                property int width3: listView.width / 22 * 3
                property int width4: listView.width / 22 * 3
                property int width5: listView.width / 22 * 1
                property int width7: listView.width / 22 * 2

                header: ColumnLayout {
                    Row {
                        Label {
                            width: listView.width0
                        }
                        Label {
                            width: listView.width1
                            text: "<b>Name</b>"
                        }
                        Label {
                            width: listView.width2
                            text: "<b>Resolution</b>"
                        }
                        Label {
                            width: listView.width3
                            text: "<b>Video Codec</b>"
                        }
                        Label {
                            width: listView.width4
                            text: "<b>Audio Codec</b>"
                        }
                        Label {
                            width: listView.width5
                            text: "<b>FPS</b>"
                        }
                        Label {
                            width: listView.width7
                            text: "<b>Filesize</b>"
                        }
                    }
                }
                delegate: ColumnLayout {
                    height: 20
                    Row {
                        CheckBox {
                            width: listView.width0
                            height: parent.height
                            checked: model.selected
                            onClicked: {
                                model.selected = checked;
                                checked = Qt.binding(function () { // restore the binding
                                    return model.selected;
                                });
                            }
                        }
                        Label {
                            width: listView.width1
                            text: model.name
                        }
                        Label {
                            width: listView.width2
                            text: model.resolution
                        }
                        Label {
                            width: listView.width3
                            text: model.vcodec
                        }
                        Label {
                            width: listView.width4
                            text: model.acodec
                        }
                        Label {
                            width: listView.width5
                            text: model.fps > 0 ? model.fps : ""
                        }
                        Label {
                            width: listView.width7
                            text: model.filesize > 0 ? (model.filesize / 2**20).toFixed(2) + " MiB" : ""
                            font.italic: model.filesize_estimate
                            color: model.filesize_estimate ? activePalette.dark : activePalette.text
                        }
                    }
                }
            }
        }

        Settings {
            property alias username: username.text
            property alias passAttribute: passAttribute.text
            property alias passValue: passValue.text
            property alias sublang: sublang.text
            property alias videoUrl: videoUrl.text
            property alias cookiesFromBrowser: cookiesFromBrowser.text
            property alias userAgent: userAgent.text
        }
    }

    Dialog {
        id: "okDialog"
        title: "mpv-ytdl-gui"
        anchors.centerIn: parent
        standardButtons: Dialog.Ok
        width: dialogLabel.implicitWidth + 15
        contentItem: Label {
            id: "dialogLabel"
        }

        Connections {
            target: mf
            function onYtdlpError() {
                dialogLabel.text = "Could not load video formats with yt-dlp."
                okDialog.open()
            }
            function onPasswordError() {
                dialogLabel.text = "Could not load password."
                okDialog.open()
            }
            function onNoSelectionError() {
                dialogLabel.text = "No formats are selected."
                okDialog.open()
            }
        }
    }

    Dialog {
        id: "settingsDialog"
        title: "Settings"
        anchors.centerIn: parent
        standardButtons: Dialog.Ok
        width: 500
        contentItem: ColumnLayout {
            TextField {
                id: "username"
                placeholderText: "Username"
                Layout.columnSpan: 1
                Layout.fillWidth: true
            }
            TextField {
                id: "sublang"
                placeholderText: "Sub-Language"
                Layout.fillWidth: true
            }
            TextField {
                id: "userAgent"
                placeholderText: "User agent"
                Layout.fillWidth: true
            }
            TextField {
                id: "cookiesFromBrowser"
                placeholderText: "Cookies from browser"
                Layout.fillWidth: true
            }
            RowLayout {
                TextField {
                    id: "passAttribute"
                    placeholderText: "Attribute"
                    Layout.fillWidth: true
                }
                TextField {
                    id: "passValue"
                    placeholderText: "Value"
                    Layout.fillWidth: true
                }
                Button {
                    text: "Load password"
                    onClicked: mf.loadPassword(passAttribute.text, passValue.text)
                    Layout.fillWidth: true
                }
            }
        }
    }
}

import QtCore
import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11

ApplicationWindow {
    id: window
    width: 1024
    height: 600
    visible: true

    function getFilesizeText(filesize, filesize_estimate) {
        if (filesize == 0) {
            return ""
        }
        let s = filesize_estimate ? "~" : "";
        if (filesize > 2**30) {
            s += (filesize / 2**30).toFixed(2) + " GiB"
        } else if (filesize > 2**20) {
            s += (filesize / 2**20).toFixed(2) + " MiB"
        } else {
            s += (filesize / 2**10).toFixed(2) + " kiB"
        }

        return s;
    }

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

                property int widthCb: 30
                property int widthBase: (listView.width - widthCb) / 30

                property int widthName: widthBase * 11
                property int widthResolution: widthBase * 4
                property int widthCodec: widthBase * 4 // (x2)
                property int widthFps: widthBase * 1
                property int widthFilesize: widthBase * 4

                header: ColumnLayout {
                    Row {
                        Label {
                            width: listView.widthCb
                        }
                        Label {
                            width: listView.widthName
                            text: "<b>Name</b>"
                        }
                        Label {
                            width: listView.widthResolution
                            text: "<b>Resolution</b>"
                        }
                        Label {
                            width: listView.widthCodec
                            text: "<b>Video Codec</b>"
                        }
                        Label {
                            width: listView.widthCodec
                            text: "<b>Audio Codec</b>"
                        }
                        Label {
                            width: listView.widthFps
                            text: "<b>FPS</b>"
                        }
                        Label {
                            width: listView.widthFilesize
                            text: "<b>Filesize</b>"
                        }
                    }
                }
                delegate: ColumnLayout {
                    height: 20
                    Row {
                        CheckBox {
                            width: listView.widthCb
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
                            width: listView.widthName
                            text: model.name
                        }
                        Label {
                            width: listView.widthResolution
                            text: model.resolution
                        }
                        Label {
                            width: listView.widthCodec
                            text: model.vcodec
                        }
                        Label {
                            width: listView.widthCodec
                            text: model.acodec
                        }
                        Label {
                            width: listView.widthFps
                            text: model.fps > 0 ? model.fps : ""
                        }
                        Label {
                            width: listView.widthFilesize
                            text: getFilesizeText(model.filesize, model.filesize_estimate)
                            font.italic: model.filesize_estimate
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

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtMultimedia 5.15
import QtQuick.Dialogs 1.3

import "home"

Window {
    visible: true

    width: 1024
    height: 768

    title: qsTr("VStream")

    Home {
        anchors.fill: parent
    }
}

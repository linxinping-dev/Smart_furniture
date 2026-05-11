import QtQuick 2.12
import QtQuick.Controls 2.5

ApplicationWindow {
    visible: true
    width: 1100
    height: 720
    title: qsTr("智能家居控制台 (Qt QML)")

    Dashboard {
        anchors.fill: parent
    }
}


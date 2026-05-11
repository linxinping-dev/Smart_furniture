import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

Rectangle {
    id: root
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#0b1020" }
        GradientStop { position: 1.0; color: "#0f172a" }
    }

    // Fixed device IDs (per your current plan)
    property string lightId: "light_01"
    property string curtainId: "curtain_01"
    property string acId: "ac_01"
    property string lockId: "lock_01"

    property string sensorsId: "sensors_01"
    property string motionId: "motion_01"

    property string clockText: ""
    property string dateText: ""

    function onlineColor(isOnline) {
        return isOnline ? "#22c55e" : "#ef4444"
    }

    // Send command wrapper
    function send(deviceType, deviceId, payload) {
        tcpClient.sendCommand(deviceType, deviceId, payload)
    }

    function formatTs(ts) {
        if (ts === undefined || ts === null || ts <= 0) return "暂无"
        var d = new Date(ts * 1000)
        return Qt.formatDateTime(d, "yyyy-MM-dd HH:mm:ss")
    }

    function pushLog(text) {
        logsModel.insert(0, { t: Qt.formatDateTime(new Date(), "HH:mm:ss"), msg: text })
        // Keep logs small and snappy
        while (logsModel.count > 60) {
            logsModel.remove(logsModel.count - 1)
        }
    }

    // Logs (connection/lastError changes)
    ListModel { id: logsModel }

    // 消息页：与服务器收发的原始行（每行一条，协议与 TcpClient 一致）
    ListModel { id: socketMsgModel }

    function pushSocketMsg(kind, line) {
        socketMsgModel.insert(0, {
            ts: Qt.formatDateTime(new Date(), "HH:mm:ss"),
            kind: kind,
            line: String(line)
        })
        while (socketMsgModel.count > 200) {
            socketMsgModel.remove(socketMsgModel.count - 1)
        }
    }

    function clearSocketMsgs() {
        while (socketMsgModel.count > 0) {
            socketMsgModel.remove(socketMsgModel.count - 1)
        }
    }

    function submitRawCmd() {
        if (!tcpClient.connected) {
            return
        }
        var t = cmdInput.text
        if (t.length < 1) {
            return
        }
        pushSocketMsg("发", t)
        tcpClient.sendRawLine(t)
    }

    Connections {
        target: tcpClient
        onConnectedChanged: {
            pushLog(tcpClient.connected ? "TCP 已连接" : "TCP 已断开")
        }
        onLastErrorChanged: {
            if (tcpClient.lastError && tcpClient.lastError.length > 0) {
                pushLog("错误: " + tcpClient.lastError)
            }
        }
        onLineReceived: {
            pushSocketMsg("收", arguments[0])
        }
    }

    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            root.clockText = Qt.formatDateTime(new Date(), "yyyy-MM-dd HH:mm:ss")
            root.dateText = Qt.formatDateTime(new Date(), "yyyy年MM月dd日 dddd")
        }
        Component.onCompleted: onTriggered()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Rectangle {
            Layout.fillWidth: true
            height: 74
            color: "#0b1220"
            radius: 12
            border.color: "#1f2a44"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 12

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Text {
                        color: "white"
                        font.pixelSize: 18
                        font.bold: true
                        text: "智能家居控制台"
                    }
                    Text {
                        color: "#94a3b8"
                        font.pixelSize: 13
                        text: root.dateText
                    }
                    Text {
                        color: "#e2e8f0"
                        font.pixelSize: 14
                        text: root.clockText
                    }
                }

                RowLayout {
                    spacing: 10
                    Rectangle {
                        width: 220
                        height: 44
                        radius: 10
                        color: "#111827"
                        border.color: "#334155"

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 10
                            Rectangle { width: 10; height: 10; radius: 5; color: onlineColor(tcpClient.connected) }
                            Text {
                                color: "white"
                                text: tcpClient.connected ? "TCP 已连接" : "TCP 未连接"
                                font.pixelSize: 14
                                elide: Text.ElideRight
                                Layout.preferredWidth: 130
                            }
                        }
                    }

                    Button {
                        id: netBtn
                        text: tcpClient.connected ? qsTr("断开") : qsTr("连接")
                        implicitHeight: 44
                        implicitWidth: 100
                        font.pixelSize: 14
                        contentItem: Text {
                            text: netBtn.text
                            font: netBtn.font
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            implicitWidth: parent.width
                            implicitHeight: parent.height
                            radius: 10
                            color: tcpClient.connected ? "#b45309" : "#15803d"
                            border.color: tcpClient.connected ? "#f59e0b" : "#22c55e"
                        }
                        onClicked: {
                            if (tcpClient.connected) {
                                tcpClient.disconnectManual()
                            } else {
                                tcpClient.connectManual()
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            TabBar {
                id: tabBar
                Layout.fillWidth: true
                currentIndex: stack.currentIndex
                TabButton { text: "首页" }
                TabButton { text: "设备" }
                TabButton { text: "日志" }
                TabButton { text: "消息" }
                onCurrentIndexChanged: stack.currentIndex = currentIndex
            }
        }

        StackLayout {
            id: stack
            Layout.fillWidth: true
            Layout.fillHeight: true

            // ---------------- Home ----------------
            Item {
                anchors.fill: parent

                GridLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    columns: 3
                    rowSpacing: 14
                    columnSpacing: 14

                    // Light tile
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 170
                        radius: 12
                        color: "#111827"
                        border.color: "#334155"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10

                            Text { color: "white"; text: "灯光"; font.bold: true; font.pixelSize: 15 }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Rectangle { width: 10; height: 10; radius: 5; color: onlineColor(homeState.lightOnline) }
                                Text { color: "#94a3b8"; text: homeState.lightOnline ? "在线" : "离线"; font.pixelSize: 12 }
                            }
                            Text {
                                color: homeState.lightPower ? "#22c55e" : "#ef4444"
                                font.pixelSize: 18
                                font.bold: true
                                text: homeState.lightPower ? "已开启" : "已关闭"
                            }
                            Text { color: "#cbd5e1"; text: "亮度: " + homeState.lightBrightness + "%"; font.pixelSize: 13 }

                            Switch {
                                checked: homeState.lightPower
                                enabled: homeState.lightOnline
                                onToggled: send("light", lightId, { "power": checked, "brightness": homeState.lightBrightness })
                                text: checked ? "开" : "关"
                            }
                        }
                    }

                    // Curtain tile
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 170
                        radius: 12
                        color: "#111827"
                        border.color: "#334155"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10

                            Text { color: "white"; text: "窗帘"; font.bold: true; font.pixelSize: 15 }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Rectangle { width: 10; height: 10; radius: 5; color: onlineColor(homeState.curtainOnline) }
                                Text { color: "#94a3b8"; text: homeState.curtainOnline ? "在线" : "离线"; font.pixelSize: 12 }
                            }
                            Text { color: "#e2e8f0"; font.pixelSize: 22; font.bold: true; text: homeState.curtainPosition + "%" }
                            ProgressBar {
                                Layout.fillWidth: true
                                from: 0; to: 100
                                value: homeState.curtainPosition
                            }
                            Text { color: "#cbd5e1"; font.pixelSize: 12; text: "开合位置" }
                        }
                    }

                    // AC tile
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 170
                        radius: 12
                        color: "#111827"
                        border.color: "#334155"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10

                            Text { color: "white"; text: "空调"; font.bold: true; font.pixelSize: 15 }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Rectangle { width: 10; height: 10; radius: 5; color: onlineColor(homeState.acOnline) }
                                Text { color: "#94a3b8"; text: homeState.acOnline ? "在线" : "离线"; font.pixelSize: 12 }
                            }
                            Text { color: homeState.acPower ? "#22c55e" : "#ef4444"; font.pixelSize: 18; font.bold: true; text: homeState.acPower ? "运行中" : "已关闭" }
                            Text { color: "#cbd5e1"; font.pixelSize: 13; text: "设定: " + homeState.acSetTemp + " C · 模式: " + homeState.acMode }

                            Switch {
                                checked: homeState.acPower
                                enabled: homeState.acOnline
                                onToggled: send("ac", acId, { "power": checked, "setTemp": homeState.acSetTemp, "mode": homeState.acMode })
                                text: checked ? "开" : "关"
                            }
                        }
                    }

                    // Lock tile
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 170
                        radius: 12
                        color: "#111827"
                        border.color: "#334155"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10

                            Text { color: "white"; text: "门锁"; font.bold: true; font.pixelSize: 15 }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Rectangle { width: 10; height: 10; radius: 5; color: onlineColor(homeState.lockOnline) }
                                Text { color: "#94a3b8"; text: homeState.lockOnline ? "在线" : "离线"; font.pixelSize: 12 }
                            }
                            Text { color: homeState.lockLocked ? "#22c55e" : "#f59e0b"; font.pixelSize: 18; font.bold: true; text: homeState.lockLocked ? "已上锁" : "已解锁" }
                            Switch {
                                checked: homeState.lockLocked
                                enabled: homeState.lockOnline
                                onToggled: send("lock", lockId, { "locked": checked })
                                text: checked ? "锁定" : "解锁"
                            }
                        }
                    }

                    // Sensors tile
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 170
                        radius: 12
                        color: "#111827"
                        border.color: "#334155"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10

                            Text { color: "white"; text: "温湿度"; font.bold: true; font.pixelSize: 15 }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Rectangle { width: 10; height: 10; radius: 5; color: onlineColor(homeState.sensorsOnline) }
                                Text { color: "#94a3b8"; text: homeState.sensorsOnline ? "在线" : "离线"; font.pixelSize: 12 }
                            }

                            Text {
                                color: "#f8fafc"
                                font.pixelSize: 26
                                font.bold: true
                                text: homeState.temperatureC.toFixed(1) + " C"
                            }

                            Text { color: "#cbd5e1"; font.pixelSize: 14; text: "湿度: " + homeState.humidity.toFixed(1) + "%" }
                        }
                    }

                    // Motion tile
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 170
                        radius: 12
                        color: "#111827"
                        border.color: "#334155"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10

                            Text { color: "white"; text: "人体感应"; font.bold: true; font.pixelSize: 15 }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Rectangle { width: 10; height: 10; radius: 5; color: onlineColor(homeState.motionOnline) }
                                Text { color: "#94a3b8"; text: homeState.motionOnline ? "在线" : "离线"; font.pixelSize: 12 }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 86
                                radius: 12
                                color: homeState.motionActive ? "#22c55e22" : "#ef444422"
                                border.color: homeState.motionActive ? "#22c55e" : "#ef4444"

                                Text {
                                    anchors.centerIn: parent
                                    color: "white"
                                    font.pixelSize: 20
                                    font.bold: true
                                    text: homeState.motionActive ? "检测到人体" : "未检测"
                                }
                            }

                            Text { color: "#cbd5e1"; font.pixelSize: 12; text: "最后触发: " + formatTs(homeState.motionLastTriggeredTs) }
                        }
                    }
                }
            }

            // ---------------- Devices ----------------
            Item {
                anchors.fill: parent

                GridLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    columns: 2
                    rowSpacing: 14
                    columnSpacing: 14

                    // ---- Light ----
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 220
                        radius: 12
                        color: "#111827"
                        border.color: "#334155"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Text { color: "white"; text: "灯光"; font.bold: true; font.pixelSize: 16 }
                                Item { Layout.fillWidth: true }
                                Rectangle { width: 10; height: 10; radius: 5; color: onlineColor(homeState.lightOnline) }
                                Text { color: "#94a3b8"; text: homeState.lightOnline ? "在线" : "离线"; font.pixelSize: 13 }
                            }

                            Switch {
                                id: lightSwitch
                                checked: homeState.lightPower
                                enabled: homeState.lightOnline
                                onToggled: send("light", lightId, { "power": checked, "brightness": Math.round(brightnessSlider.value) })
                                text: checked ? "电源: 开" : "电源: 关"
                            }

                            Text {
                                color: "#cbd5e1"
                                text: "亮度: " + Math.round(brightnessSlider.value) + "%"
                                font.pixelSize: 14
                            }

                            Slider {
                                id: brightnessSlider
                                from: 0; to: 100
                                value: homeState.lightBrightness
                                enabled: homeState.lightOnline && lightSwitch.checked
                                onValueChanged: {
                                    if (pressed) {
                                        send("light", lightId, { "power": lightSwitch.checked, "brightness": Math.round(value) })
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Button { text: "10%"; enabled: homeState.lightOnline; onClicked: send("light", lightId, { "power": true, "brightness": 10 }) }
                                Button { text: "50%"; enabled: homeState.lightOnline; onClicked: send("light", lightId, { "power": true, "brightness": 50 }) }
                                Button { text: "100%"; enabled: homeState.lightOnline; onClicked: send("light", lightId, { "power": true, "brightness": 100 }) }
                            }
                        }
                    }

                    // ---- Curtain ----
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 220
                        radius: 12
                        color: "#111827"
                        border.color: "#334155"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Text { color: "white"; text: "窗帘"; font.bold: true; font.pixelSize: 16 }
                                Item { Layout.fillWidth: true }
                                Rectangle { width: 10; height: 10; radius: 5; color: onlineColor(homeState.curtainOnline) }
                                Text { color: "#94a3b8"; text: homeState.curtainOnline ? "在线" : "离线"; font.pixelSize: 13 }
                            }

                            Text { color: "#cbd5e1"; text: "开合: " + homeState.curtainPosition + "%"; font.pixelSize: 14 }

                            Slider {
                                from: 0; to: 100
                                value: homeState.curtainPosition
                                enabled: homeState.curtainOnline
                                onValueChanged: {
                                    if (pressed) send("curtain", curtainId, { "position": Math.round(value) })
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Button { text: "0%"; enabled: homeState.curtainOnline; onClicked: send("curtain", curtainId, { "position": 0 }) }
                                Button { text: "50%"; enabled: homeState.curtainOnline; onClicked: send("curtain", curtainId, { "position": 50 }) }
                                Button { text: "100%"; enabled: homeState.curtainOnline; onClicked: send("curtain", curtainId, { "position": 100 }) }
                            }
                        }
                    }

                    // ---- AC ----
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 220
                        radius: 12
                        color: "#111827"
                        border.color: "#334155"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Text { color: "white"; text: "空调"; font.bold: true; font.pixelSize: 16 }
                                Item { Layout.fillWidth: true }
                                Rectangle { width: 10; height: 10; radius: 5; color: onlineColor(homeState.acOnline) }
                                Text { color: "#94a3b8"; text: homeState.acOnline ? "在线" : "离线"; font.pixelSize: 13 }
                            }

                            Switch {
                                id: acPowerSwitch
                                checked: homeState.acPower
                                enabled: homeState.acOnline
                                onToggled: send("ac", acId, { "power": checked, "setTemp": Math.round(tempSlider.value), "mode": modeCombo.currentText })
                                text: checked ? "电源: 开" : "电源: 关"
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Text { color: "#cbd5e1"; text: "设定温度:"; font.pixelSize: 14 }
                                Text { color: "#f8fafc"; text: Math.round(tempSlider.value) + " C"; font.pixelSize: 14 }
                                Item { Layout.fillWidth: true }
                            }

                            Slider {
                                id: tempSlider
                                from: 16; to: 30
                                stepSize: 1
                                value: homeState.acSetTemp
                                enabled: homeState.acOnline && acPowerSwitch.checked
                                onValueChanged: {
                                    if (pressed) {
                                        send("ac", acId, { "power": acPowerSwitch.checked, "setTemp": Math.round(value), "mode": modeCombo.currentText })
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Button { text: "-1"; enabled: homeState.acOnline && acPowerSwitch.checked; onClicked: { var v = Math.max(16, Math.round(tempSlider.value - 1)); send("ac", acId, { "power": true, "setTemp": v, "mode": modeCombo.currentText }) } }
                                Button { text: "+1"; enabled: homeState.acOnline && acPowerSwitch.checked; onClicked: { var v = Math.min(30, Math.round(tempSlider.value + 1)); send("ac", acId, { "power": true, "setTemp": v, "mode": modeCombo.currentText }) } }
                                Item { Layout.fillWidth: true }
                            }

                            ComboBox {
                                id: modeCombo
                                enabled: homeState.acOnline && acPowerSwitch.checked
                                model: [ "auto", "cool", "heat" ]
                                currentIndex: {
                                    var m = homeState.acMode
                                    if (m === "cool") return 1
                                    if (m === "heat") return 2
                                    return 0
                                }
                                onCurrentTextChanged: {
                                    if (acPowerSwitch.checked && homeState.acOnline) {
                                        send("ac", acId, { "power": acPowerSwitch.checked, "setTemp": Math.round(tempSlider.value), "mode": currentText })
                                    }
                                }
                            }
                        }
                    }

                    // ---- Lock ----
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 220
                        radius: 12
                        color: "#111827"
                        border.color: "#334155"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Text { color: "white"; text: "门锁"; font.bold: true; font.pixelSize: 16 }
                                Item { Layout.fillWidth: true }
                                Rectangle { width: 10; height: 10; radius: 5; color: onlineColor(homeState.lockOnline) }
                                Text { color: "#94a3b8"; text: homeState.lockOnline ? "在线" : "离线"; font.pixelSize: 13 }
                            }

                            Switch {
                                id: lockSwitch
                                checked: homeState.lockLocked
                                enabled: homeState.lockOnline
                                onToggled: send("lock", lockId, { "locked": checked })
                                text: lockSwitch.checked ? "状态: 已上锁" : "状态: 已解锁"
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Button { text: "上锁"; enabled: homeState.lockOnline; onClicked: send("lock", lockId, { "locked": true }) }
                                Button { text: "解锁"; enabled: homeState.lockOnline; onClicked: send("lock", lockId, { "locked": false }) }
                            }
                        }
                    }

                    // ---- Sensors ----
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 220
                        radius: 12
                        color: "#111827"
                        border.color: "#334155"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Text { color: "white"; text: "温湿度"; font.bold: true; font.pixelSize: 16 }
                                Item { Layout.fillWidth: true }
                                Rectangle { width: 10; height: 10; radius: 5; color: onlineColor(homeState.sensorsOnline) }
                                Text { color: "#94a3b8"; text: homeState.sensorsOnline ? "在线" : "离线"; font.pixelSize: 13 }
                            }

                            Text { color: "#f8fafc"; font.pixelSize: 28; font.bold: true; text: homeState.temperatureC.toFixed(1) + " C" }
                            ProgressBar { Layout.fillWidth: true; from: 0; to: 50; value: homeState.temperatureC; enabled: false }
                            Text { color: "#cbd5e1"; font.pixelSize: 14; text: "湿度: " + homeState.humidity.toFixed(1) + "%" }
                            ProgressBar { Layout.fillWidth: true; from: 0; to: 100; value: homeState.humidity; enabled: false }
                        }
                    }

                    // ---- Motion ----
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 220
                        radius: 12
                        color: "#111827"
                        border.color: "#334155"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Text { color: "white"; text: "人体感应"; font.bold: true; font.pixelSize: 16 }
                                Item { Layout.fillWidth: true }
                                Rectangle { width: 10; height: 10; radius: 5; color: onlineColor(homeState.motionOnline) }
                                Text { color: "#94a3b8"; text: homeState.motionOnline ? "在线" : "离线"; font.pixelSize: 13 }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 110
                                radius: 12
                                color: homeState.motionActive ? "#22c55e22" : "#ef444422"
                                border.color: homeState.motionActive ? "#22c55e" : "#ef4444"

                                Text {
                                    anchors.centerIn: parent
                                    color: "white"
                                    font.pixelSize: 26
                                    font.bold: true
                                    text: homeState.motionActive ? "检测到人体" : "未检测"
                                }
                            }

                            Text { color: "#cbd5e1"; font.pixelSize: 13; text: "最后触发: " + formatTs(homeState.motionLastTriggeredTs) }
                        }
                    }
                }
            }

            // ---------------- Logs ----------------
            Item {
                anchors.fill: parent

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 14
                    color: "#111827"
                    border.color: "#334155"
                    radius: 12

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 10

                        Text { color: "white"; text: "运行日志"; font.bold: true; font.pixelSize: 16 }

                        ListView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            model: logsModel
                            clip: true
                            delegate: Rectangle {
                                width: ListView.view.width
                                height: 30
                                color: "#0b1220"
                                border.color: "#1f2a44"
                                border.width: 1

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 10
                                    Text { text: model.t; color: "#94a3b8"; font.pixelSize: 12; width: 70 }
                                    Text { text: model.msg; color: "#e2e8f0"; font.pixelSize: 13; elide: Text.ElideRight }
                                }
                            }
                        }
                    }
                }
            }

            // ---------------- 消息（原始收发） ----------------
            Item {
                anchors.fill: parent

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 14
                    color: "#111827"
                    border.color: "#334155"
                    radius: 12

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 10

                        Text {
                            color: "white"
                            text: "服务器消息"
                            font.bold: true
                            font.pixelSize: 16
                        }
                        Text {
                            color: "#94a3b8"
                            font.pixelSize: 12
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                            text: "每行一条消息（换行分隔）。发送时若未以换行结尾会自动补一行。需先点顶部「连接」。Enter 发送，Shift+Enter 换行。"
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: "#0b1220"
                            radius: 10
                            border.color: "#1f2a44"

                            ListView {
                                id: socketMsgList
                                anchors.fill: parent
                                anchors.margins: 8
                                clip: true
                                spacing: 8
                                model: socketMsgModel

                                delegate: Item {
                                    width: socketMsgList.width
                                    height: msgCol.height + 10

                                    Column {
                                        id: msgCol
                                        width: parent.width - 4
                                        x: 2
                                        spacing: 4

                                        Text {
                                            width: msgCol.width
                                            color: "#64748b"
                                            font.pixelSize: 11
                                            text: model.ts + "  ·  " + model.kind
                                        }
                                        Text {
                                            width: msgCol.width
                                            wrapMode: Text.Wrap
                                            color: model.kind === "发" ? "#93c5fd" : "#e2e8f0"
                                            font.pixelSize: 13
                                            font.family: "Consolas"
                                            text: model.line
                                        }
                                    }
                                }

                                ScrollBar.vertical: ScrollBar {
                                    policy: ScrollBar.AsNeeded
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10

                            TextArea {
                                id: cmdInput
                                Layout.fillWidth: true
                                Layout.preferredHeight: 88
                                placeholderText: "在此输入要发送的内容（如一行 JSON）…"
                                wrapMode: TextArea.Wrap
                                color: "#e2e8f0"
                                background: Rectangle {
                                    radius: 8
                                    color: "#0b1220"
                                    border.color: "#334155"
                                }
                                selectByMouse: true
                                Keys.onPressed: {
                                    if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                                        if (!(event.modifiers & Qt.ShiftModifier)) {
                                            event.accepted = true
                                            submitRawCmd()
                                        }
                                    }
                                }
                            }

                            ColumnLayout {
                                spacing: 8
                                Button {
                                    id: sendRawBtn
                                    text: "发送"
                                    implicitWidth: 88
                                    enabled: tcpClient.connected && cmdInput.text.length > 0
                                    onClicked: submitRawCmd()
                                }
                                Button {
                                    text: "清空记录"
                                    implicitWidth: 88
                                    onClicked: clearSocketMsgs()
                                }
                                Button {
                                    text: "清空输入"
                                    implicitWidth: 88
                                    onClicked: cmdInput.text = ""
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


#include "TcpClient.h"

#include "HomeState.h"

#include <QAbstractSocket>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDateTime>

TcpClient::TcpClient(QObject* parent) : QObject(parent) {
    connect(&m_socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
    connect(&m_socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(&m_socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
    connect(&m_socket,
            QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this,
            &TcpClient::onErrorOccurred);

    m_reconnectTimer.setInterval(3000);
    m_reconnectTimer.setSingleShot(true);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &TcpClient::onReconnectTimeout);
}

void TcpClient::setServerAddress(const QString& host, quint16 port) {
    m_host = host;
    m_port = port;
}

void TcpClient::connectManual() {
    m_manualDisconnect = false;
    connectToServer(m_host, m_port);
}

void TcpClient::disconnectManual() {
    m_manualDisconnect = true;
    m_reconnectTimer.stop();
    m_socket.abort();
}

void TcpClient::connectToServer(const QString& host, quint16 port) {
    m_host = host;
    m_port = port;

    if (m_socket.state() == QAbstractSocket::ConnectedState || m_socket.state() == QAbstractSocket::ConnectingState) {
        return;
    }

    m_lastError.clear();
    emit lastErrorChanged();

    m_socket.connectToHost(m_host, m_port);
}

void TcpClient::onConnected() {
    updateConnected(true);
    m_reconnectTimer.stop();
}

void TcpClient::onDisconnected() {
    updateConnected(false);
    if (!m_manualDisconnect) {
        tryReconnect();
    }
}

void TcpClient::onErrorOccurred(QAbstractSocket::SocketError socketError) {
    (void)socketError;
    m_lastError = m_socket.errorString();
    emit lastErrorChanged();
}

void TcpClient::tryReconnect() {
    if (m_manualDisconnect) {
        return;
    }
    if (m_host.isEmpty() || m_port == 0) {
        return;
    }
    if (m_socket.state() == QAbstractSocket::ConnectedState || m_socket.state() == QAbstractSocket::ConnectingState) {
        return;
    }
    m_reconnectTimer.start();
}

void TcpClient::onReconnectTimeout() {
    if (m_manualDisconnect) {
        return;
    }
    if (m_host.isEmpty() || m_port == 0) {
        return;
    }
    if (m_socket.state() == QAbstractSocket::ConnectedState || m_socket.state() == QAbstractSocket::ConnectingState) {
        return;
    }
    m_lastError.clear();
    emit lastErrorChanged();
    m_socket.connectToHost(m_host, m_port);
}

void TcpClient::updateConnected(bool connected) {
    if (m_connected == connected) {
        return;
    }
    m_connected = connected;
    emit connectedChanged();
}

void TcpClient::sendJsonLine(const QJsonObject& obj) {
    if (m_socket.state() != QAbstractSocket::ConnectedState) {
        return;
    }

    const QJsonDocument doc(obj);
    QByteArray line = doc.toJson(QJsonDocument::Compact);
    line.append('\n'); // protocol: one JSON per line

    m_socket.write(line);
    m_socket.flush();
}

void TcpClient::sendRawLine(const QString& text) {
    if (m_socket.state() != QAbstractSocket::ConnectedState) {
        return;
    }

    QByteArray data = text.toUtf8();
    if (!data.endsWith('\n')) {
        data.append('\n');
    }
    m_socket.write(data);
    m_socket.flush();
}

void TcpClient::sendCommand(const QString& deviceType, const QString& deviceId, const QVariantMap& payload) {
    QJsonObject root;
    root["type"] = "command";
    root["deviceType"] = deviceType;
    root["deviceId"] = deviceId;
    root["requestId"] = QStringLiteral("r%1").arg(++m_requestCounter);
    root["timestamp"] = static_cast<qint64>(QDateTime::currentSecsSinceEpoch());

    QJsonObject payloadObj;
    for (auto it = payload.constBegin(); it != payload.constEnd(); ++it) {
        payloadObj.insert(it.key(), QJsonValue::fromVariant(it.value()));
    }
    root["payload"] = payloadObj;

    sendJsonLine(root);
}

void TcpClient::applyStatusBatch(const QJsonObject& obj) {
    if (!m_homeState) {
        return;
    }

    const QJsonArray payloadArr = obj.value(QStringLiteral("payload")).toArray();
    for (const QJsonValue& v : payloadArr) {
        if (!v.isObject()) {
            continue;
        }
        const QJsonObject item = v.toObject();
        const QString deviceType = item.value(QStringLiteral("deviceType")).toString();
        const QString deviceId = item.value(QStringLiteral("deviceId")).toString();
        (void)deviceId;

        const QJsonObject payloadObj = item.value(QStringLiteral("payload")).toObject();

        if (deviceType == QLatin1String("light")) {
            const bool online = payloadObj.value(QStringLiteral("online")).toBool(false);
            const bool power = payloadObj.value(QStringLiteral("power")).toBool(false);
            const int brightness = payloadObj.value(QStringLiteral("brightness")).toInt(0);
            m_homeState->setLightStatus(online, power, brightness);
        } else if (deviceType == QLatin1String("curtain")) {
            const bool online = payloadObj.value(QStringLiteral("online")).toBool(false);
            const int position = payloadObj.value(QStringLiteral("position")).toInt(0);
            m_homeState->setCurtainStatus(online, position);
        } else if (deviceType == QLatin1String("ac")) {
            const bool online = payloadObj.value(QStringLiteral("online")).toBool(false);
            const bool power = payloadObj.value(QStringLiteral("power")).toBool(false);
            const int setTemp = payloadObj.value(QStringLiteral("setTemp")).toInt(26);
            const QString mode = payloadObj.value(QStringLiteral("mode")).toString(QStringLiteral("auto"));
            m_homeState->setAcStatus(online, power, setTemp, mode);
        } else if (deviceType == QLatin1String("lock")) {
            const bool online = payloadObj.value(QStringLiteral("online")).toBool(false);
            const bool locked = payloadObj.value(QStringLiteral("locked")).toBool(false);
            m_homeState->setLockStatus(online, locked);
        } else if (deviceType == QLatin1String("sensors")) {
            const bool online = payloadObj.value(QStringLiteral("online")).toBool(false);
            const double t = payloadObj.value(QStringLiteral("temperatureC")).toDouble(0.0);
            const double h = payloadObj.value(QStringLiteral("humidity")).toDouble(0.0);
            m_homeState->setSensorsStatus(online, t, h);
        } else if (deviceType == QLatin1String("motion")) {
            const bool online = payloadObj.value(QStringLiteral("online")).toBool(false);
            const bool active = payloadObj.value(QStringLiteral("motion")).toBool(false);
            const qint64 lastTriggeredTs = payloadObj.value(QStringLiteral("lastTriggeredTs")).toVariant().toLongLong();
            m_homeState->setMotionStatus(online, active, lastTriggeredTs);
        }
    }
}

void TcpClient::onReadyRead() {
    m_buffer += m_socket.readAll();

    int idx = -1;
    while ((idx = m_buffer.indexOf('\n')) != -1) {
        QByteArray line = m_buffer.left(idx).trimmed();
        m_buffer.remove(0, idx + 1);

        if (line.isEmpty()) {
            continue;
        }

        emit lineReceived(QString::fromUtf8(line));

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(line, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            continue;
        }

        const QJsonObject obj = doc.object();
        if (obj.value(QStringLiteral("type")).toString() == QLatin1String("statusBatch")) {
            applyStatusBatch(obj);
        }
    }
}


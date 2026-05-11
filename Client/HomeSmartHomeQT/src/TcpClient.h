#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QJsonObject>
#include <QVariantMap>

class HomeState;

class TcpClient : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit TcpClient(QObject* parent = nullptr);

    void setServerAddress(const QString& host, quint16 port);
    Q_INVOKABLE void connectManual();
    Q_INVOKABLE void disconnectManual();

    bool isConnected() const { return m_connected; }
    QString lastError() const { return m_lastError; }

    // Exposed to QML
    Q_INVOKABLE void sendCommand(const QString& deviceType, const QString& deviceId, const QVariantMap& payload);
    Q_INVOKABLE void sendRawLine(const QString& text);

signals:
    void connectedChanged();
    void lastErrorChanged();
    void lineReceived(const QString& line);

public slots:
    void setHomeState(HomeState* state) { m_homeState = state; }

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void onReconnectTimeout();

private:
    void updateConnected(bool connected);
    void tryReconnect();
    void sendJsonLine(const QJsonObject& obj);
    void connectToServer(const QString& host, quint16 port);
    void applyStatusBatch(const QJsonObject& root);

    QTcpSocket m_socket;
    QTimer m_reconnectTimer;
    QByteArray m_buffer;

    QString m_host;
    quint16 m_port = 0;
    bool m_manualDisconnect = false;
    bool m_connected = false;
    QString m_lastError;
    quint32 m_requestCounter = 0;

    HomeState* m_homeState = nullptr; // non-owning
};


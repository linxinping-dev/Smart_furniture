#pragma once

#include <QObject>

class HomeState : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool lightOnline READ lightOnline NOTIFY lightOnlineChanged)
    Q_PROPERTY(bool lightPower READ lightPower NOTIFY lightPowerChanged)
    Q_PROPERTY(int lightBrightness READ lightBrightness NOTIFY lightBrightnessChanged)

    Q_PROPERTY(bool curtainOnline READ curtainOnline NOTIFY curtainOnlineChanged)
    Q_PROPERTY(int curtainPosition READ curtainPosition NOTIFY curtainPositionChanged)

    Q_PROPERTY(bool acOnline READ acOnline NOTIFY acOnlineChanged)
    Q_PROPERTY(bool acPower READ acPower NOTIFY acPowerChanged)
    Q_PROPERTY(int acSetTemp READ acSetTemp NOTIFY acSetTempChanged)
    Q_PROPERTY(QString acMode READ acMode NOTIFY acModeChanged)

    Q_PROPERTY(bool lockOnline READ lockOnline NOTIFY lockOnlineChanged)
    Q_PROPERTY(bool lockLocked READ lockLocked NOTIFY lockLockedChanged)

    Q_PROPERTY(bool sensorsOnline READ sensorsOnline NOTIFY sensorsOnlineChanged)
    Q_PROPERTY(double temperatureC READ temperatureC NOTIFY temperatureCChanged)
    Q_PROPERTY(double humidity READ humidity NOTIFY humidityChanged)

    Q_PROPERTY(bool motionOnline READ motionOnline NOTIFY motionOnlineChanged)
    Q_PROPERTY(bool motionActive READ motionActive NOTIFY motionActiveChanged)
    Q_PROPERTY(qint64 motionLastTriggeredTs READ motionLastTriggeredTs NOTIFY motionLastTriggeredTsChanged)

public:
    explicit HomeState(QObject* parent = nullptr);

    bool lightOnline() const { return m_lightOnline; }
    bool lightPower() const { return m_lightPower; }
    int lightBrightness() const { return m_lightBrightness; }

    bool curtainOnline() const { return m_curtainOnline; }
    int curtainPosition() const { return m_curtainPosition; }

    bool acOnline() const { return m_acOnline; }
    bool acPower() const { return m_acPower; }
    int acSetTemp() const { return m_acSetTemp; }
    QString acMode() const { return m_acMode; }

    bool lockOnline() const { return m_lockOnline; }
    bool lockLocked() const { return m_lockLocked; }

    bool sensorsOnline() const { return m_sensorsOnline; }
    double temperatureC() const { return m_temperatureC; }
    double humidity() const { return m_humidity; }

    bool motionOnline() const { return m_motionOnline; }
    bool motionActive() const { return m_motionActive; }
    qint64 motionLastTriggeredTs() const { return m_motionLastTriggeredTs; }

    // Called by TcpClient when a statusBatch arrives.
    void setLightStatus(bool online, bool power, int brightness);
    void setCurtainStatus(bool online, int position);
    void setAcStatus(bool online, bool power, int setTemp, const QString& mode);
    void setLockStatus(bool online, bool locked);
    void setSensorsStatus(bool online, double temperatureC, double humidity);
    void setMotionStatus(bool online, bool active, qint64 lastTriggeredTs);

signals:
    void lightOnlineChanged();
    void lightPowerChanged();
    void lightBrightnessChanged();

    void curtainOnlineChanged();
    void curtainPositionChanged();

    void acOnlineChanged();
    void acPowerChanged();
    void acSetTempChanged();
    void acModeChanged();

    void lockOnlineChanged();
    void lockLockedChanged();

    void sensorsOnlineChanged();
    void temperatureCChanged();
    void humidityChanged();

    void motionOnlineChanged();
    void motionActiveChanged();
    void motionLastTriggeredTsChanged();

private:
    bool m_lightOnline = false;
    bool m_lightPower = false;
    int m_lightBrightness = 0;

    bool m_curtainOnline = false;
    int m_curtainPosition = 0;

    bool m_acOnline = false;
    bool m_acPower = false;
    int m_acSetTemp = 26;
    QString m_acMode = QStringLiteral("auto");

    bool m_lockOnline = false;
    bool m_lockLocked = false;

    bool m_sensorsOnline = false;
    double m_temperatureC = 0.0;
    double m_humidity = 0.0;

    bool m_motionOnline = false;
    bool m_motionActive = false;
    qint64 m_motionLastTriggeredTs = 0;
};


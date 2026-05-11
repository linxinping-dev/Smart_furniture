#include "HomeState.h"

HomeState::HomeState(QObject* parent) : QObject(parent) {}

void HomeState::setLightStatus(bool online, bool power, int brightness) {
    bool changed = false;

    if (m_lightOnline != online) {
        m_lightOnline = online;
        emit lightOnlineChanged();
        changed = true;
    }
    if (m_lightPower != power) {
        m_lightPower = power;
        emit lightPowerChanged();
        changed = true;
    }
    if (m_lightBrightness != brightness) {
        m_lightBrightness = brightness;
        emit lightBrightnessChanged();
        changed = true;
    }
    (void)changed;
}

void HomeState::setCurtainStatus(bool online, int position) {
    if (m_curtainOnline != online) {
        m_curtainOnline = online;
        emit curtainOnlineChanged();
    }
    if (m_curtainPosition != position) {
        m_curtainPosition = position;
        emit curtainPositionChanged();
    }
}

void HomeState::setAcStatus(bool online, bool power, int setTemp, const QString& mode) {
    if (m_acOnline != online) {
        m_acOnline = online;
        emit acOnlineChanged();
    }
    if (m_acPower != power) {
        m_acPower = power;
        emit acPowerChanged();
    }
    if (m_acSetTemp != setTemp) {
        m_acSetTemp = setTemp;
        emit acSetTempChanged();
    }
    if (m_acMode != mode) {
        m_acMode = mode;
        emit acModeChanged();
    }
}

void HomeState::setLockStatus(bool online, bool locked) {
    if (m_lockOnline != online) {
        m_lockOnline = online;
        emit lockOnlineChanged();
    }
    if (m_lockLocked != locked) {
        m_lockLocked = locked;
        emit lockLockedChanged();
    }
}

void HomeState::setSensorsStatus(bool online, double temperatureC, double humidity) {
    if (m_sensorsOnline != online) {
        m_sensorsOnline = online;
        emit sensorsOnlineChanged();
    }
    if (m_temperatureC != temperatureC) {
        m_temperatureC = temperatureC;
        emit temperatureCChanged();
    }
    if (m_humidity != humidity) {
        m_humidity = humidity;
        emit humidityChanged();
    }
}

void HomeState::setMotionStatus(bool online, bool active, qint64 lastTriggeredTs) {
    if (m_motionOnline != online) {
        m_motionOnline = online;
        emit motionOnlineChanged();
    }
    if (m_motionActive != active) {
        m_motionActive = active;
        emit motionActiveChanged();
    }

    if (m_motionLastTriggeredTs != lastTriggeredTs) {
        m_motionLastTriggeredTs = lastTriggeredTs;
        emit motionLastTriggeredTsChanged();
    }
}


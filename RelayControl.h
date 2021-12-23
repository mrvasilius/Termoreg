#pragma once

#ifndef RELAY_BUS
#define RELAY_BUS D0
#endif

#define RELAY_ON 1
#define RELAY_OFF 0

class RelayControl
{
public:
    enum cMode
    {
        C_OFF,
        C_ON,
        C_AUTO
    };
    RelayControl();
    void setRelay(int theTemp, float temperature);
    void setRelay(int theTemp, float minTemperature, float maxTemperature);
    bool getStatus();
    cMode getControlMode();
    void setControlMode(cMode controlMode);

private:
    cMode _controlMode = C_AUTO;
    bool _relayStatus = RELAY_OFF;
    void setRelayOn();
    void setRelayOff();
};

extern RelayControl RC;

RelayControl::RelayControl() {
    pinMode(RELAY_BUS, OUTPUT);
}

void RelayControl::setRelay(int theTemp, float temperature)
{
    setRelay(theTemp, temperature, temperature);
}

void RelayControl::setRelay(int theTemp, float minTemperature, float maxTemperature)
{
    switch (_controlMode)
    {
    case C_ON:
        setRelayOn();
        break;

    case C_OFF:
        setRelayOff();
        break;

    case C_AUTO:
        if ((minTemperature < theTemp) && _relayStatus == RELAY_OFF)
        {
            setRelayOn();
        }
        if ((maxTemperature > theTemp) && _relayStatus == RELAY_ON)
        {
            setRelayOff();
        }
        break;

    default:
        break;
    }
}

bool RelayControl::getStatus()
{
    return _relayStatus;
}

RelayControl::cMode RelayControl::getControlMode()
{
    return _controlMode;
}

void RelayControl::setControlMode(cMode controlMode)
{
    _controlMode = controlMode;
}

void RelayControl::setRelayOn()
{
    _relayStatus = RELAY_ON;
    digitalWrite(RELAY_BUS, RELAY_ON);
}

void RelayControl::setRelayOff()
{
    _relayStatus = RELAY_OFF;
    digitalWrite(RELAY_BUS, RELAY_OFF);
}

RelayControl RC = RelayControl();
#pragma once

#include "GyverTimer.h"
#include "GyverEncoder.h"
#include "RelayControl.h"
#include "TuningTemp.h"
#include "DisplayShow.h"

#ifndef ENC_S1
#define ENC_S1 D6
#endif

#ifndef ENC_S2
#define ENC_S2 D5
#endif

#ifndef ENC_KEY
#define ENC_KEY D4
#endif

class ModeControl
{
public:
    enum eMode
    {
        SHOW_TEMP_MODE,
        EDIT_TEMP_MODE,
        EDIT_CONTROL_MODE,
        EDIT_SENS_MODE
    };
    ModeControl();
    void init(RelayControl &rc, int *theTemp, float *temperature, int *qtySens, int *actualSense, void (*updateDto)());
    void loop();
    void setMode(eMode editMode);

private:
    RelayControl *_rc;
    int *_theTemp;
    float *_temperature;
    int *_qtySens;
    int *_actualSens;
    void (*_updateDto)();
    eMode _editMode = SHOW_TEMP_MODE;
    Encoder _enc{ENC_S2, ENC_S1, ENC_KEY, TYPE2};
    GTimer_ms _editTimer{10000};
    void showTempMode();
    void editTempMode();
    void editControlMode();
    void editSensMode();
};

extern ModeControl MC;

ModeControl::ModeControl() {}

void ModeControl::init(RelayControl &rc, int *theTemp, float *temperature, int *qtySens, int *actualSens, void (*updateDto)())
{
    _editTimer.setMode(MANUAL);
    _enc.setTickMode(AUTO);
    _rc = &rc;
    _theTemp = theTemp;
    _temperature = temperature;
    _qtySens = qtySens;
    _actualSens = actualSens;
    _updateDto = updateDto;
    Tuning.init(_theTemp);
}

void ModeControl::loop()
{
    switch (_editMode)
    {
    case SHOW_TEMP_MODE:
        showTempMode();
        break;

    case EDIT_TEMP_MODE:
        editTempMode();
        break;

    case EDIT_CONTROL_MODE:
        editControlMode();
        break;

    case EDIT_SENS_MODE:
        editSensMode();
        break;

    default:
        break;
    }
}

void ModeControl::setMode(eMode editMode)
{
    _editMode = editMode;
    _editTimer.start();
    _editTimer.reset();
}

void ModeControl::showTempMode()
{
    Display.showTemp(*_temperature, _rc->getControlMode() == RelayControl::cMode::C_AUTO);
    if (_enc.isTurn())
    {
        _editMode = EDIT_TEMP_MODE;
        _editTimer.start();
        _enc.resetStates();
    }
    if (_enc.isClick())
    {
        _editTimer.start();
        _editMode = EDIT_CONTROL_MODE;
    }
}

void ModeControl::editTempMode()
{
    Display.showEdit(*_theTemp);

    if (_enc.isTurn())
    {
        _rc->setControlMode(RelayControl::cMode::C_AUTO);
        _editTimer.reset();
    }

    if (_enc.isRight())
    {
        Tuning.raiseTemp();
    }

    if (_enc.isLeft())
    {
        Tuning.reduceTemp();
    }

    if (_enc.isClick())
    {
        _editTimer.reset();
        _editMode = EDIT_CONTROL_MODE;
    }

    if (_editTimer.isReady())
    {
        _rc->setRelay(*_theTemp, *_temperature);
        _editTimer.reset();
        _editTimer.stop();
        _editMode = SHOW_TEMP_MODE;
        _updateDto();
    }
}

void ModeControl::editControlMode()
{
    switch (_rc->getControlMode())
    {
    case RelayControl::cMode::C_ON:
        Display.showModeOn();
        if (_enc.isRight())
        {
            _rc->setControlMode(RelayControl::cMode::C_OFF);
        }
        if (_enc.isLeft())
        {
            _rc->setControlMode(RelayControl::cMode::C_ON);
        }
        break;

    case RelayControl::cMode::C_OFF:
        Display.showModeOff();
        if (_enc.isRight())
        {
            _rc->setControlMode(RelayControl::cMode::C_AUTO);
        }
        if (_enc.isLeft())
        {
            _rc->setControlMode(RelayControl::cMode::C_ON);
        }
        break;

    case RelayControl::cMode::C_AUTO:
        Display.showModeAuto();
        if (_enc.isRight())
        {
            _rc->setControlMode(RelayControl::cMode::C_AUTO);
        }
        if (_enc.isLeft())
        {
            _rc->setControlMode(RelayControl::cMode::C_OFF);
        }
        break;

    default:
        break;
    }
    if (_enc.isTurn())
    {
        _editTimer.reset();
        _rc->setRelay(*_theTemp, *_temperature);
    }

    if (_enc.isClick())
    {
        _editMode = EDIT_SENS_MODE;
    }

    if (_editTimer.isReady())
    {
        _editTimer.reset();
        _editTimer.stop();
        _editMode = SHOW_TEMP_MODE;
        _rc->setRelay(*_theTemp, *_temperature);
        _updateDto();
    }
}

void ModeControl::editSensMode()
{
    Display.showSensor(*_actualSens);

    if (_enc.isTurn())
    {
        _editTimer.reset();
    }

    if (_enc.isRight() && *_actualSens < *_qtySens - 1)
    {
        *_actualSens++;
    }

    if (_enc.isLeft() && *_actualSens > 0)
    {
        *_actualSens--;
    }

    if (_editTimer.isReady() || _enc.isClick())
    {
        _editTimer.reset();
        _editTimer.stop();
        _editMode = SHOW_TEMP_MODE;
        _updateDto();
    }
}

ModeControl MC = ModeControl();
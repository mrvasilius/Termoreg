#pragma once

#include "GyverTM1637.h"
#include "GyverTimer.h"

#ifndef DISP_CLK
#define DISP_CLK D1
#endif

#ifndef DISP_DIO
#define DISP_DIO D3
#endif

class DisplayShow : public GyverTM1637
{
public:
    DisplayShow();
    void showTemp(float temperature, bool isAuto);
    void showEdit(int theTemp);
    void showModeOn();
    void showModeOff();
    void showModeAuto();
    void showSensor(int sensor);
    void showError(byte code);

private:
    //массив изображений 7 сегментных символов от 0 до 9 и пустота
    const byte simv[10] = {
        B00111111, //0
        B00000110, //1
        B01011011, //2
        B01001111, //3
        B01100110, //4
        B01101101, //5
        B01111101, //6
        B00000111, //7
        B01111111, //8
        B01101111, //9
    };

    GTimer_ms _dispTimer;
    void show(byte dig1, byte dig2, byte dig3, byte dig4);
};

extern DisplayShow Display;

DisplayShow::DisplayShow() : GyverTM1637(DISP_CLK, DISP_DIO) {
    _dispTimer.setInterval(100);
    clear();
    brightness(7); // яркость, 0 - 7 (минимум - максимум)
}

void DisplayShow::showTemp(float temperature, bool isAuto)
{
    int _temperature = (int)temperature;
    byte dig3;
    if (_temperature >= 10)
    {
        dig3 = simv[_temperature / 10];
    }
    else if (_temperature < 0)
    {
        dig3 = _dash;
    }
    else
    {
        dig3 = _empty;
    }

    byte dig4 = simv[_temperature % 10];

    show(dig3, dig4, _degree, isAuto ? _A : _empty);
}

void DisplayShow::showEdit(int theTemp)
{
    byte dig3 = (theTemp >= 10) ? simv[theTemp / 10] : _empty;
    byte dig4 = simv[theTemp % 10];

    show(_E, _d, dig3, dig4);
}

void DisplayShow::showModeOn()
{
    show(_O, _n, _empty, _empty);
}

void DisplayShow::showModeOff()
{
    show(_O, _f, _f, _empty);
}

void DisplayShow::showModeAuto()
{
    show(_A, _u, _t, _o);
}

void DisplayShow::showSensor(int sensor)
{
    if (sensor > 9 || sensor < 0)
    {
        show(_S, _empty, _E, _r);
    }
    show(_S, simv[sensor], _empty, _empty);
}

void DisplayShow::showError(byte code)
{
    if (code > 9 || code < 0)
    {
        show(_E, _r, _r, _empty);
    }
    show(_E, _r, _r, simv[code]);
}

void DisplayShow::show(byte dig1, byte dig2, byte dig3, byte dig4)
{
    if (_dispTimer.isReady())
    {
        point(false);
        displayByte(dig1, dig2, dig3, dig4);
    }
}

DisplayShow Display = DisplayShow();
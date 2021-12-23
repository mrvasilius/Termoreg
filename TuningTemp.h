#pragma once

#ifndef MIN_TEMP
#define MIN_TEMP 0
#endif

#ifndef MAX_TEMP
#define MAX_TEMP 30
#endif

class TuningTemp
{
public:
    TuningTemp();
    TuningTemp(int *theTemp);
    void init(int *theTemp);
    void raiseTemp();
    void reduceTemp();
    void setTemp(int newTemp);

private:
    int *_theTemp;
};

extern TuningTemp Tuning;

TuningTemp::TuningTemp() {}
TuningTemp::TuningTemp(int *theTemp)
{
    init(theTemp);
}

void TuningTemp::init(int *theTemp)
{
    _theTemp = theTemp;
}

void TuningTemp::raiseTemp()
{
    *_theTemp = *_theTemp < MAX_TEMP ? *_theTemp + 1 : MAX_TEMP;
}
void TuningTemp::reduceTemp()
{
    *_theTemp = *_theTemp > MIN_TEMP ? *_theTemp - 1 : MIN_TEMP;
}

void TuningTemp::setTemp(int newTemp)
{
    if (newTemp > MAX_TEMP)
        newTemp = MAX_TEMP;
    if (newTemp < MIN_TEMP)
        newTemp = MIN_TEMP;
    *_theTemp = newTemp;
}

TuningTemp Tuning = TuningTemp();
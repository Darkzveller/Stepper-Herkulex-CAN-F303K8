#include <mbed.h>
#include "STEPPER.h"

DigitalOut STBY(D7);
DigitalOut STEP(D6);
DigitalOut DIr(D3);
DigitalOut EN(A2);
DigitalOut M0(D5);
DigitalOut M1(D9);
DigitalOut M2(D8);
DigitalIn FDC(PA_1);

void initStepper(void)
{
  STBY = 1;
  EN = 0;
  M0 = 0;
  M1 = 0;
  M2 = 0;
}

void blockStepper(void)
{
  STBY = 1;
  EN = 1;
  M0 = 1;
  M1 = 1;
  M2 = 1;
}

int stepper(int swpulse, int m0, int m1, int m2, int dir, int dur, bool up )
{
  M0 = m0;
  M1 = m1;
  M2 = m2;
  DIr = dir;
  EN = 1;
  // step generator
  for (int i = 0; i < swpulse; i++)
  {
    STEP = 1;
    ThisThread::sleep_for(2ms);
    STEP = 0;
    ThisThread::sleep_for(2ms);
    if ((FDC.read()) == 1 && up)
    {
      EN = 0;
      return 1;
    }
  }
  EN = 0;
  return 0;
}
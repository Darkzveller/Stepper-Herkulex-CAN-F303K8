#include <mbed.h>
#include "STEPPER.h"

// CAN can(PA_11, PA_12, 1000000);  // CAN Rx pin name, CAN Tx pin name
DigitalOut led(LED1);

int main()
{
    double temps = 0;
    initStepper();
    printf("CRAC stm32f303k8 démo\n");

    while (true)
    {
        stepper(610,0,0,0,1,0,0);

        ThisThread::sleep_for(250ms);
        led = 0;
        ThisThread::sleep_for(250ms);
        led = 1;
        printf("%.1d ", Caca);

        printf("%.1f\n", temps);
        temps += 0.5;
    }
}

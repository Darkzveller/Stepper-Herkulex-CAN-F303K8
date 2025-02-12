#include <Arduino.h>
#include <HERKULEX.h>
bool activate_detect = false;

void setup()
{
    Serial.begin(115200);
    init_serial_1_for_herkulex();
}
void loop()
{
    if (Serial.available() > 0)
    {
        char c = Serial.read();

        if (c == 's')
        {
            Serial.println("Scanning...");
            Serial.println("Addresses are displayed in hexadecimal");
            activate_detect = true;
        }
    }

    if (detect_id(activate_detect) == 253)
    {
        Serial.println("jsp = 0");
        activate_detect = false;
    }
}


// #include <mbed.h>
// #include "STEPPER.h"
// // CAN can(PA_11, PA_12, 1000000);  // CAN Rx pin name, CAN Tx pin name
// DigitalOut led(LED1);

// int main()
// {
//     double temps = 0;
//     initStepper();
//     printf("CRAC stm32f303k8 démo\n");

//     while (true)
//     {
//         stepper(convert_angle_to_pas(90), PAS_COMPLET, 0);

//         ThisThread::sleep_for(250ms);
//         led = 0;
//         ThisThread::sleep_for(250ms);
//         led = 1;
//         stepper(convert_angle_to_pas(-90), PAS_COMPLET, 0);
//         ThisThread::sleep_for(250ms);
//         led = 0;
//         ThisThread::sleep_for(250ms);
//         led = 1;
//         printf("%.1f\n", temps);

//         temps += 0.5;
//     }
// }

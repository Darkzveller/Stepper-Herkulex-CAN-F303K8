#include <Arduino.h>
#include <HERKULEX.h>
#include <STEPPER.h>
#include <CAN.h>

bool activate_detect = false;
int id_msg_can_rx; // ID des msg CAN recu
char data_msg_can_rx[8]; // datas du msg CAN Reçu
int i;
// CAN can(PA_11, PA_12, 1000000); // CAN Rx pin name, CAN Tx pin name

void setup()
{
    Serial.begin(115200);
    Serial.println("Serial.begin");
    delay(100);
    initStepper();
    Serial.println("initStepper()");

    init_serial_1_for_herkulex();
    delay(100);
    Serial.println("init_serial_1_for_herkulex");

    setup_can();
    delay(100);
    Serial.println("setup_can");
    delay(100);
}
void loop()
{
    
    // Serial.println("Fonctionnement");

    // stepper(convert_angle_to_pas(90), PAS_COMPLET, 0);
    // delay(250);
    // stepper(convert_angle_to_pas(-90), PAS_COMPLET, 0);
    // delay(250);

    // test_herkulex();

    // sendCANMessage(0xFF,0x11,0x22,0,0,0,0,0,0);
    // delay(250);
    if(receiveCANMessage(&id_msg_can_rx, data_msg_can_rx)){
        if(id_msg_can_rx == HERKULEX_AIMANT_CENTRE){
            cmd_aimant_centre(data_msg_can_rx[0]); // met le mouvement demandé
        }
        if(id_msg_can_rx == HERKULEX_AIMANT_COTE){
            cmd_aimant_cote(data_msg_can_rx[0]); // met le mouvement demandé
        }
        if(id_msg_can_rx == HERKULEX_PIVOT_COTE){
            if(data_msg_can_rx[0]==CENTRE){
                aimant_cote_centre();
            }
            else if(data_msg_can_rx[0]==COTE){
                aimant_cote_cote();
            }
            else if(data_msg_can_rx[0]==ECARTER){
                aimant_cote_ecarter();
            }
        }
    }

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

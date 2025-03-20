#include <Arduino.h>
#include <HERKULEX.h>
#include <STEPPER.h>
#include <CAN.h>
#include <STM32FreeRTOS.h>

SemaphoreHandle_t mutex = NULL; // handle du mutex

bool activate_detect = false;
int id_msg_can_rx;       // ID des msg CAN recu
char data_msg_can_rx[8]; // datas du msg CAN Reçu
int i = 0;
// CAN can(PA_11, PA_12, 1000000); // CAN Rx pin name, CAN Tx pin name
void Gestion_STEPPER(void *parametres);
void Gestion_CAN(void *parametres);

void setup()
{
    portBASE_TYPE s1;

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

    mutex = xSemaphoreCreateMutex(); // cree le mutex
    s1 = xTaskCreate(Gestion_STEPPER, "Gestion_STEPPER", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(Gestion_CAN, "Gestion_CAN", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    // check for creation errors
    if (s1 != pdPASS)
    {
        Serial.println(F("Creation problem"));
        while (1)
            ;
    }

    // start scheduler
    vTaskStartScheduler(); // lance le scheduler
    // Affiche si y'a un problème de mémoire
    Serial.println("Insufficient RAM");
    while (1)
        ;
}

void loop()
{
    // loop vide
}

void Gestion_CAN(void *parametres)
{
    while (1)
    {
        // prend le mutex avant d'utiliser les periphériques séries
        if (xSemaphoreTake(mutex, (TickType_t)5) == pdTRUE)
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

            // regarde si on a un msg can pour nous
            if (receiveCANMessage(&id_msg_can_rx, data_msg_can_rx))
            {
                Serial.print("msg recu ");
                Serial.println(id_msg_can_rx);
                switch (id_msg_can_rx)
                {
                case HERKULEX_AVANT_AIMANT_CENTRE:
                    cmd_aimant_centre(data_msg_can_rx[0]); // met le mouvement demandé
                    break;
                case HERKULEX_AVANT_AIMANT_COTE:
                    cmd_aimant_cote(data_msg_can_rx[0]); // met le mouvement demandé
                    break;
                case HERKULEX_AVANT_PIVOT_COTE:
                    if (data_msg_can_rx[0] == CENTRE)
                    {
                        aimant_cote_centre();
                    }
                    else if (data_msg_can_rx[0] == COTE)
                    {
                        aimant_cote_cote();
                    }
                    else if (data_msg_can_rx[0] == ECARTER)
                    {
                        aimant_cote_ecarter();
                    }
                    break;
                default:
                    break;
                }
            }
            
            xSemaphoreGive(mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(5)); // tache périodique de 5 ms
    }
}

void Gestion_STEPPER(void *parametres)
{
    while (1)
    {
        // stepper(100, PAS_COMPLET, 0);
        // stepper(-100, PAS_COMPLET, 0);
        // prend le mutex avant d'utiliser les periphériques séries
        // if (xSemaphoreTake(mutex, (TickType_t)5) == pdTRUE)
        // {
        //     Serial.println("stepper");
        //     xSemaphoreGive(mutex);
        // }
        vTaskDelay(pdMS_TO_TICKS(5)); // tache périodique de 5 ms
    }
}
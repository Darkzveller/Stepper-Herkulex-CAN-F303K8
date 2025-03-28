#include <Arduino.h>
#include <HERKULEX.h>
#include <STEPPER.h>
#include <CAN.h>
#include <STM32FreeRTOS.h>
#include <CONFIG_CARTE.h>
// offset pour différencier les deux cartes MPP, avant, ou arrière


SemaphoreHandle_t mutex = NULL; // handle du mutex

bool activate_detect = false;
int id_msg_can_rx;       // ID des msg CAN recu
char data_msg_can_rx[8]; // datas du msg CAN Reçu
int i = 0;

// nbre de pas
int nb_step;
// mode fdc ou non, actionner : 1 = actionner les MPP, 0 = ne pas les actionner
bool mode_fdc, actionner = 0;

void Gestion_STEPPER(void *parametres);
void Gestion_CAN(void *parametres);

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

    mutex = xSemaphoreCreateMutex(); // cree le mutex
    xTaskCreate(Gestion_STEPPER, "Gestion_STEPPER", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(Gestion_CAN, "Gestion_CAN", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    // check for creation errors

    // start scheduler
    sendCANMessage(BOOT_CARTE_MPP, 1, 0, 0, 0, 0, 0, 0, 0); // signale que la carte a boot, pret à recevoir des ordres
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

// Reçoit les trames CAN et prend ceux qui concernent la carte et les traite
void Gestion_CAN(void *parametres)
{
    
    while (1)
    {
        // prend le mutex avant d'utiliser les periphériques séries
        if (xSemaphoreTake(mutex, (TickType_t)5) == pdTRUE)
        {
            // regarde si on a un msg can pour nous
            if (receiveCANMessage(&id_msg_can_rx, data_msg_can_rx))
            {
                Serial.print("msg recu 0x");
                Serial.println(id_msg_can_rx, HEX);
                switch (id_msg_can_rx)
                {
                case HERKULEX_AIMANT_CENTRE:
                    cmd_aimant_centre(data_msg_can_rx[0]); // met le mouvement demandé
                    break;
                case HERKULEX_AIMANT_COTE:
                    cmd_aimant_cote(data_msg_can_rx[0]); // met le mouvement demandé
                    break;
                case HERKULEX_PIVOT_COTE:
                    // met le mouvement demandé par la trame
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
                case CMD_MPP:
                    // nbre de pas codé sur les 4 premiers octet de la trame
                    nb_step = *((int*)&data_msg_can_rx);
                    // mode de fdc sur l'octet de 4
                    mode_fdc = data_msg_can_rx[4];
                    actionner = 1; // dis à la tache stepper de actionner le MPP
                    Serial.println(nb_step);
                    break;
                case HERKULEX_PIVOT_PINCE:
                    cmd_pivot_pince(data_msg_can_rx[0]);
                    break;
                case HERKULEX_PINCE:
                    cmd_pince(data_msg_can_rx[0]);
                    break;
                default:
                    break;
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
            xSemaphoreGive(mutex); // rend le mutex
        }
        vTaskDelay(pdMS_TO_TICKS(5)); // tache périodique de 5 ms
    }
}

void Gestion_STEPPER(void *parametres)
{
    while (1)
    {
        // si on demande d'actionner avec le CAN
        if(actionner == 1){
            Serial.println("stepper");
            stepper(nb_step, PAS_COMPLET, mode_fdc);
            actionner = 0; // a finit d'actionner le MPP
        }

        // prend le mutex avant d'utiliser les periphériques séries
        // if (xSemaphoreTake(mutex, (TickType_t)5) == pdTRUE)
        // {
        //     Serial.println("stepper");
        //     xSemaphoreGive(mutex);
        // }
        vTaskDelay(pdMS_TO_TICKS(5)); // tache périodique de 5 ms
    }
}


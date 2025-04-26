#include <Arduino.h>
#include <HERKULEX.h>
#include <STEPPER.h>
#include <CAN.h>
#include <STM32FreeRTOS.h>
#include <CONFIG_CARTE.h>
// offset pour différencier les deux cartes MPP, avant, ou arrière

#define PIN_POMPE PF0 // Pin pour la pompe

SemaphoreHandle_t mutex = NULL;        // handle du mutex
TaskHandle_t build_handle = nullptr;   // handle de la contruction
TaskHandle_t stepper_handle = nullptr; // handle du stepper

bool activate_detect = false;
int id_msg_can_rx;       // ID des msg CAN recu
char data_msg_can_rx[8]; // datas du msg CAN Reçu
int i = 0;

// nbre de pas
int nb_step;
// mode fdc ou non, flag_stepper : 1 = MPP actionné, 0 = pas actionné
bool mode_fdc, actionner = 0, flag_stepper = 0; 

void Gestion_STEPPER(void *parametres);
void Gestion_CAN(void *parametres);
void build_floor2(void *);
void cmd_pompe(bool mouvement);
// void task_interrupt_stepper(void*);

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

    pinMode(PIN_POMPE,OUTPUT);

    mutex = xSemaphoreCreateMutex(); // cree le mutex
    xTaskCreate(Gestion_STEPPER, "Gestion_STEPPER", configMINIMAL_STACK_SIZE, NULL, 2, &stepper_handle);
    xTaskCreate(Gestion_CAN, "Gestion_CAN", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(build_floor2, "bluid_floor2", configMINIMAL_STACK_SIZE, NULL, 2, &build_handle);
    // xTaskCreate(task_interrupt_stepper, "task_interrupt_stepper", configMINIMAL_STACK_SIZE, NULL, 3, &stepper_handle);
    // check for creation errors

    // start scheduler
    sendCANMessage(BOOT_CARTE_MPP, 1, 0, 0, 0, 0, 0, 0, 0); // signale que la carte a boot, pret à recevoir des ordres
    vTaskStartScheduler();                                  // lance le scheduler

    // Affiche si y'a un problème de mémoire

    Serial.println("Insufficient RAM");
    while (1)
        ;
}

void loop()
{
    // loop vide
}

void cmd_pompe(bool mouvement){
    if(mouvement == ATTRAPER) digitalWrite(PIN_POMPE,HIGH);
    else digitalWrite(PIN_POMPE,LOW);
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
                        aimant_cote_attraper();
                    }
                    else if (data_msg_can_rx[0] == ECARTER)
                    {
                        aimant_cote_ecarter();
                    }
                    break;
                case CMD_MPP:
                    // nbre de pas codé sur les 4 premiers octet de la trame
                    nb_step = *((int *)&data_msg_can_rx);
                    // mode de fdc sur l'octet de 4
                    mode_fdc = data_msg_can_rx[4];
                    xTaskNotifyGive(stepper_handle); // on lance la tâche
                    Serial.println(nb_step);
                    break;
                case HERKULEX_PIVOT_PINCE:
                    cmd_pivot_pince(data_msg_can_rx[0]);
                    break;
                case HERKULEX_PINCE:
                    cmd_pince(data_msg_can_rx[0]);
                    break;
                case CONTRUIRE_PREPARER:
                    cmd_pivot_pince(AVANT_CONSTRUCTION); // déploit pivot pince
                    aimant_cote_attraper();            // pivots des côtés
                    // met à la pos pour attraper les cannetes
                    cmd_aimant_centre(ATTRAPER);
                    cmd_aimant_cote(ATTRAPER);
                    cmd_pince(RETIRER);
                    break;
                case CONSTRUIRE_2ETAGE:
                    /*
                        L'avantage de cette méthode c'est qu'on bouffe rien
                        En gros -> pas de mutex rien
                        On fait en quelque sorte une interruption tâche
                    */
                    xTaskNotifyGive(build_handle); // on lance la tâche
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

// tache qui actionne le pas à pas
void Gestion_STEPPER(void *parametres) // v1
{
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        stepper(nb_step, PAS_COMPLET, mode_fdc);
        blockStepper();
        flag_stepper = 0;

    }
}

// tache qui permet de constuire un étage
void build_floor2(void *)
{
    /*
        L'idée c'est de ne pas bloquer le reste du code d'où la tache
        La tache exe les mouvements dans l'odre du switch case
        La tache vérifie les mouvemements des herkulex en permanence
        Toutes les variables sont locales comme ça !
    */

    bool building = true;    // pour la boucle de construction
    int8_t step_2_build = 0; // pour le switch case de construction*

    // position du pivot de la pince
    short pos_pivot_mid = 0;

    // position de la pince
    short pos_pince = 0;

    // positions des deux pivots des côtés dans une liste
    short pos_pivot_gauche;
    short pos_pivot_droit;

    // positions des servo des aimants
    short pos_aimant_centre;
    short pos_aimant_droit;
    short pos_aimant_gauche;
    /*
        pos_pivots_sides[0] = Pivot_gauche
        pos_pivots_sides[1] = Pivot_droit
    */

    // on atteint que ça soit à un pour continuer
    bool stepper_finish = 0;

    unsigned long last_update = 0; // Stocke le temps de la dernière mise à jour
    unsigned long now = 0;         // Stocke le temps actuel

    char data_stepper_msg[8];

    while (true)
    {   
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // met à jour la valeur de last_update
        now = millis();
        last_update = now;
        while (building)
        {
            now = millis();
            switch (step_2_build)
            {
                /*
                    Je suis parti du principe qu'on peut faire 3 herkulex
                    en même temps
                */

            case 0:                                  // pas besoin de default comme on commence qu'une fois réveillé
                // attrape la planche et ecarte les aimants pour monter plus tard
                cmd_pivot_pince(DEPLOYER);
                cmd_pompe(ATTRAPER);
                aimant_cote_ecarter();
                
                if((now - last_update) > 2000){
                    Serial.println(now - last_update);
                    // étape suivante dis au MPP de monter
                    last_update = now;
                    nb_step = 1, mode_fdc = 1, flag_stepper = 1;
                    xTaskNotifyGive(stepper_handle);
                    step_2_build = 1;
                }
                break;
            case 1:
                // quand le MPP aura finit il pass à l'étape d'après
                if(flag_stepper == 0){
                    // le MPP a finit de monter
                    Serial.println("next step");
                    step_2_build = 2;
                    last_update = now;
                }
                break;
            case 2:
                // remet les cannettes sur le coté
                aimant_cote_attraper();
                if(now - last_update > 2000){
                    step_2_build = 3;
                    last_update = now;
                }
                break;
            case 3:
                building = false;
                Serial.println("finished building");
                break;
            }

            vTaskDelay(pdMS_TO_TICKS(5)); // tache qui se répète toutes les 5ms
        }

        step_2_build = 0;
        building = true;
    }
}

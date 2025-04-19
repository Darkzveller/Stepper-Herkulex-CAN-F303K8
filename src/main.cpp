#include <Arduino.h>
#include <HERKULEX.h>
#include <STEPPER.h>
#include <CAN.h>
#include <STM32FreeRTOS.h>
#include <CONFIG_CARTE.h>
// offset pour différencier les deux cartes MPP, avant, ou arrière


SemaphoreHandle_t mutex = NULL; // handle du mutex
TaskHandle_t build_handle = nullptr; // handle de la contruction
TaskHandle_t stepper_handle = nullptr; // handle du stepper

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
void build_floor2(void*);
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

    mutex = xSemaphoreCreateMutex(); // cree le mutex
    xTaskCreate(Gestion_STEPPER, "Gestion_STEPPER", configMINIMAL_STACK_SIZE, NULL, 2, &stepper_handle);
    xTaskCreate(Gestion_CAN, "Gestion_CAN", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(build_floor2, "bluid_floor2", configMINIMAL_STACK_SIZE, NULL, 2, &build_handle);
    // xTaskCreate(task_interrupt_stepper, "task_interrupt_stepper", configMINIMAL_STACK_SIZE, NULL, 3, &stepper_handle);
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
            // display_servo_position();
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
                    xTaskNotifyGive(stepper_handle); // on lance la tâche 
                    Serial.println(nb_step);
                    break;
                case HERKULEX_PIVOT_PINCE:
                    cmd_pivot_pince(data_msg_can_rx[0]);
                    break;
                case HERKULEX_PINCE:
                    cmd_pince(data_msg_can_rx[0]);
                    break;
                // case CONSTRUIRE_ETAGE:
                /*
                    L'avantage de cette méthode c'est qu'on bouffe rien 
                    En gros -> pas de mutex rien
                    On fait en quelque sorte une interruption tâche
                */
                    // xTaskNotifyGive(build_handle) // on lance la tâche
                    // break;
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
void Gestion_STEPPER(void *parametres) //v1
{
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // si on demande d'actionner avec le CAN
        // Serial.print("stepper");
        // Serial.println(nb_step);
        stepper(nb_step, PAS_COMPLET, mode_fdc);
        blockStepper();


        // prend le mutex avant d'utiliser les periphériques séries
        // if (xSemaphoreTake(mutex, (TickType_t)5) == pdTRUE)
        // {
        //     Serial.println("stepper");
        //     xSemaphoreGive(mutex);
        // }
        // vTaskDelay(pdMS_TO_TICKS(5)); // tache périodique de 5 ms
    }
}

// void task_interrupt_stepper(void*){ //v2 test
//     /*
//         Même principe que build_2_floor on fait une task interruption pour éviter
//         l'utilisation au plus des mutex et pour faire en sorte que la tâche ne 
//         mange rien tant qu'elle n'est pas envoyée

//         J'ai prévu qu'on puisse lui envoyer un peu tout que ça soit du CAN ou juste
//         un message classique comme ce qu'on a dans build_floor2
//     */

//     int local_nb_step = 0; // valeur locale des pas
//     bool local_fdc_mode = false; // valeure locale du mode
//     uint32_t data_swooper; // sorte de pointeur pour aller chercher la data

//     while(true){
//         // on attend la notif et on prend la data rx qui nous est envoyé
//         xTaskNotifyWait(0, 0xFFFFFFFF, &data_swooper, portMAX_DELAY);
//         // méthode qui devrait être efficace mais obligé d'avoir des "uint"

//         char* local_data = (char*)data_swooper; 
//         // on peut donc traiter le message rx de manière locale ! 

//         // nbre de pas codé sur les 4 premiers octet de la trame
//         local_nb_step = *((int*)local_data);
//         // mode de fdc sur l'octet de 4
//         local_fdc_mode = local_data[4];

//         stepper(local_nb_step, PAS_COMPLET, local_fdc_mode);
//         blockStepper();
        
//         local_nb_step = 0;
//         local_fdc_mode = false;

//         // comme pour la tâche build_floor2 une fois que c'est terminé : sleep sleep
//     }
// }

// tache qui permet de constuire un étage
void build_floor2(void*){
    /*
        L'idée c'est de ne pas bloquer le reste du code d'où la tache
        La tache exe les mouvements dans l'odre du switch case
        La tache vérifie les mouvemements des herkulex en permanence
        Toutes les variables sont locales comme ça ! 
    */

    bool building = true; // pour la boucle de construction
    int8_t step_2_build = 0; // pour le switch case de construction*

    // position du pivot de la pince
    int16_t pos_pivot_mid = 0; 

    // position de la pince
    int16_t pos_pince = 0;

    // positions des deux pivots des côtés dans une liste
    int16_t pos_pivots_sides[2];
    /*
        pos_pivots_sides[0] = Pivot_gauche
        pos_pivots_sides[1] = Pivot_droit
    */ 

    // on atteint que ça soit à un pour continuer 
    bool stepper_finish = 0;

    while(true){

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        while(building){
            switch (step_2_build)
            {
                /*
                    Je suis parti du principe qu'on peut faire 3 herkulex
                    en même temps 
                */

            case 0: // pas besoin de default comme on commence qu'une fois réveillé
                cmd_pivot_pince(DEPLOYER); // déploit pivot pince
                aimant_cote_ecarter(); // pivots des côtés
                step_2_build = 1;
                break;
            case 1: 
                // on attend d'arriver au bon endroit pour le pivot
                pos_pivot_mid = get_servo_pos(Pivot_pince);
                // on vérifie si on est bon sur le décalage des boîtes
                pos_pivots_sides[0] = get_servo_pos(Pivot_gauche);
                pos_pivots_sides[1] = get_servo_pos(Pivot_droit);
                
                if(pos_pivots_sides[0] == 512 + ANGLE_PIVOT_COTE_ECARTER / 0.325 &&
                    pos_pivots_sides[1] == 512 - ANGLE_PIVOT_COTE_ECARTER / 0.325 &&
                    pos_pivot_mid == (512 + 90) / 0.325
                ){
                    cmd_pince(ATTRAPER);
                    step_2_build = 2;
                }
                break;
            case 2:
                // on vérifie la position de la pince
                pos_pince = get_servo_pos(Pince);
                //j'ai un doute sur la position de la pince
                if(pos_pince == 512 + ANGLE_PINCE_ATTRAPER / 0.325){
                    // on dit hé oh réveille toi et prends le tableau !
                    // c'est comme si c'était une fct avec un argument
                    // xTaskNotify(stepper_handle,(uint32_t)data_stepper_msg,eSetValueWithOverwrite);
                    step_2_build = 3;
                } 
                break;
            case 3:
                //on lit ce que la fct stepper nous renvoit
                stepper_finish = stepper(50000,PAS_COMPLET,1);
                if(stepper_finish){
                    blockStepper();
                    aimant_cote_cote();
                    step_2_build = 4;
                }
                break;
            case 4:
                pos_pivots_sides[0] = get_servo_pos(Pivot_gauche);
                pos_pivots_sides[1] = get_servo_pos(Pivot_droit);

                if(pos_pivots_sides[0] == 512 + ANGLE_PIVOT_COTE_ECARTER / 0.325 &&
                    pos_pivots_sides[1] == 512 - ANGLE_PIVOT_COTE_ECARTER / 0.325
                ) {
                    // unblockStepper() //comme je ne sais pas totalement son fct je mets en commentaire mais dès que je peux je fais la fonction
                    step_2_build = 5;
                }
                break;
            case 5: // on a finit 
                building = false;
                break;
            }

            vTaskDelay(pdMS_TO_TICKS(5)); // tache qui se répète toutes les 5ms
        }

        step_2_build = 0;
        building = true;

    }

}


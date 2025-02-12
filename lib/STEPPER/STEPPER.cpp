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

int stepper(int swpulse, int microstep, bool up )
{
    // Déterminer le sens de rotation en fonction de swpulse
    if (swpulse > 0) {
        DIr = 1;  // Sens avant
    } else if (swpulse < 0) {
        DIr = 0;  // Sens arrière
        swpulse = -swpulse; // Rendre positif pour la boucle
    } else {
        return 0; // Ne rien faire si swpulse == 0
    }

    // Configurer la résolution de micro-pas
    M0 = microstep & 0b001;
    M1 = (microstep >> 1) & 0b001;
    M2 = (microstep >> 2) & 0b001;

    EN = 1;  // Activation du moteur

    // Génération des impulsions STEP
    for (int i = 0; i < swpulse; i++)
    {
        STEP = 1;
        ThisThread::sleep_for(2ms);
        STEP = 0;
        ThisThread::sleep_for(2ms);

        // Vérification du capteur fin de course si up est activé
        if ((FDC.read()) == 1 && up)
        {
            EN = 0;
            return 1; // Arrêt prématuré si le capteur est déclenché
        }
    }

    EN = 0; // Désactivation du moteur après la séquence
    return 0;
}

int convert_angle_to_pas(int angle_deg){

  int pas = PAS_PAR_TOUR * angle_deg/360.0;
  return pas; 
}
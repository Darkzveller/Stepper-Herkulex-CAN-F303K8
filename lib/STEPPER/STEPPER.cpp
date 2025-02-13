#include <Arduino.h>
#include "STEPPER.h"

// Définition des broches
#define PIN_STBY D7
#define PIN_STEP D6
#define PIN_DIr D3
#define PIN_EN A2
#define PIN_M0 PB5
#define PIN_M1 D9
#define PIN_M2 D8
#define PIN_FDC PA_1

// Variables de contrôle moteur
bool STBY, STEP, DIr, EN, M0, M1, M2;

void initStepper()
{
  // Initialisation des valeurs
  STBY = 1;
  EN = 0;
  M0 = 0;
  M1 = 0;
  M2 = 0;

  // Configuration des broches en mode OUTPUT
  pinMode(PIN_STBY, OUTPUT);
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIr, OUTPUT);
  pinMode(PIN_EN, OUTPUT);
  pinMode(PIN_M0, OUTPUT);
  pinMode(PIN_M1, OUTPUT);
  pinMode(PIN_M2, OUTPUT);
  pinMode(PIN_FDC, INPUT);

  // Appliquer les valeurs initiales
  digitalWrite(PIN_STBY, STBY);
  digitalWrite(PIN_STEP, STEP);
  digitalWrite(PIN_DIr, DIr);
  digitalWrite(PIN_EN, EN);
  digitalWrite(PIN_M0, M0);
  digitalWrite(PIN_M1, M1);
  digitalWrite(PIN_M2, M2);
}

void blockStepper()
{
  // Mise en veille du moteur
  STBY = 1;
  EN = 1;
  M0 = 1;
  M1 = 1;
  M2 = 1;

  digitalWrite(PIN_STBY, STBY);
  digitalWrite(PIN_EN, EN);
  digitalWrite(PIN_M0, M0);
  digitalWrite(PIN_M1, M1);
  digitalWrite(PIN_M2, M2);
}

// Fonction pour faire avancer le moteur pas à pas
int stepper(int swpulse, int microstep, bool up)
{
  if (swpulse > 0)
  {
    DIr = 1; // Sens avant
  }
  else if (swpulse < 0)
  {
    DIr = 0;            // Sens arrière
    swpulse = -swpulse; // Rendre positif pour la boucle
  }
  else
  {
    return 0; // Ne rien faire si swpulse == 0
  }

  // Configurer le microstepping
  M0 = microstep & 0b001;
  M1 = (microstep >> 1) & 0b01;
  M2 = (microstep >> 2) & 0b01;

  EN = 1;
  digitalWrite(PIN_EN, EN);
  digitalWrite(PIN_DIr, DIr);
  digitalWrite(PIN_M0, M0);
  digitalWrite(PIN_M1, M1);
  digitalWrite(PIN_M2, M2);

  for (int i = 0; i < swpulse; i++)
  {
    STEP = 1;
    digitalWrite(PIN_STEP, STEP);
    delayMicroseconds(2000);
    STEP = 0;
    digitalWrite(PIN_STEP, STEP);
    delayMicroseconds(2000);

    // Arrêt si fin de course détectée
    if (digitalRead(PIN_FDC) == 1 && up)
    {
      EN = 0;
      digitalWrite(PIN_EN, EN);
      return 1;
    }
  }

  EN = 0;
  digitalWrite(PIN_EN, EN);
  return 0;
}

// Fonction pour convertir un angle en nombre de pas
int convert_angle_to_pas(int angle_deg)
{

  int pas = PAS_PAR_TOUR * angle_deg / 360.0;
  return pas;
}
/*#include <Arduino.h>
#include "STEPPER.h"

// DigitalOut STBY(D7);
// DigitalOut STEP(D6);
// DigitalOut DIr(D3);
// DigitalOut EN(A2);
// DigitalOut M0(D5);
// DigitalOut M1(D9);
// DigitalOut M2(D8);
// DigitalIn FDC(PA_1);
#define PIN_STBY D7 // D7
#define PIN_STEP D6 // D6
#define PIN_DIr  D3  // D3
#define PIN_EN   A2   // A2
#define PIN_M0   D5   // D5
#define PIN_M1   D9   // D9
#define PIN_M2   D8   // D8
#define PIN_FDC  PA_1  // PA_1

int8_t STBY = 1;
int8_t STEP = 0;
int8_t DIr = 0;
int8_t EN = 0;
int8_t M0 = 0;
int8_t M1 = 0;
int8_t M2 = 0;
int8_t FDC = 0;

void initStepper(void)
{
  STBY = 1;
  EN = 0;
  M0 = 0;
  M1 = 0;
  M2 = 0;
  pinMode(PIN_STBY, OUTPUT);
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIr, OUTPUT);
  pinMode(PIN_EN, OUTPUT);
  pinMode(PIN_M0, OUTPUT);
  pinMode(PIN_M1, OUTPUT);
  pinMode(PIN_M2, OUTPUT);
  pinMode(PIN_FDC, INPUT);

  digitalWrite(PIN_STBY, STBY);
  digitalWrite(PIN_STEP, STEP);
  digitalWrite(PIN_DIr, DIr);
  digitalWrite(PIN_EN, EN);
  digitalWrite(PIN_M0, M0);
  digitalWrite(PIN_M1, M1);
  digitalWrite(PIN_M2, M2);
}

void blockStepper(void)
{
  STBY = 1;
  EN = 1;
  M0 = 1;
  M1 = 1;
  M2 = 1;
  digitalWrite(PIN_STBY, STBY);
  digitalWrite(PIN_STEP, STEP);
  digitalWrite(PIN_DIr, DIr);
  digitalWrite(PIN_EN, EN);
  digitalWrite(PIN_M0, M0);
  digitalWrite(PIN_M1, M1);
  digitalWrite(PIN_M2, M2);
}

int stepper(int swpulse, int microstep, bool up)
{
  // Déterminer le sens de rotation en fonction de swpulse
  if (swpulse > 0)
  {
    DIr = 1; // Sens avant
  }
  else if (swpulse < 0)
  {
    DIr = 0;            // Sens arrière
    swpulse = -swpulse; // Rendre positif pour la boucle
  }
  else
  {
    return 0; // Ne rien faire si swpulse == 0
  }

  // Configurer la résolution de micro-pas
  M0 = microstep & 0b001;
  M1 = (microstep >> 1) & 0b001;
  M2 = (microstep >> 2) & 0b001;

  EN = 1; // Activation du moteur

  // Génération des impulsions STEP
  for (int i = 0; i < swpulse; i++)
  {
    STEP = 1;
    delay(2);
    STEP = 0;
    delay(2);

    // Vérification du capteur fin de course si up est activé
    if ((digitalRead(PIN_FDC)) == 1 && up)
    {
      EN = 0;
      return 1; // Arrêt prématuré si le capteur est déclenché
    }

    digitalWrite(PIN_STEP, STEP);
    digitalWrite(PIN_DIr, DIr);
    digitalWrite(PIN_EN, EN);
    digitalWrite(PIN_M0, M0);
    digitalWrite(PIN_M1, M1);
    digitalWrite(PIN_M2, M2);
  }

  EN = 0; // Désactivation du moteur après la séquence
  digitalWrite(PIN_EN, EN);

  return 0;
}

int convert_angle_to_pas(int angle_deg)
{

  int pas = PAS_PAR_TOUR * angle_deg / 360.0;
  return pas;
}
*/
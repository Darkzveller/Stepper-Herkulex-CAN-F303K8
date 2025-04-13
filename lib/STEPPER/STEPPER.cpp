#include <Arduino.h>
#include <STEPPER.h>
#include <STM32FreeRTOS.h>

// Définition des broches
// #define PIN_STBY D7
// #define PIN_STEP D6
// #define PIN_DIR D3
// #define PIN_EN A2
// #define PIN_M0 PB5
// #define PIN_M1 D9
// #define PIN_M2 D8
// #define PIN_FDC_HAUT PA1
// #define PIN_FDC_BAS PA4

#define PIN_STBY PA5
#define PIN_STEP PA4
#define PIN_DIR PA3
#define PIN_EN PB1
#define PIN_M0 PB0
#define PIN_M1 PA7
#define PIN_M2 PA6
#define PIN_FDC_HAUT PA1
#define PIN_FDC_BAS PA8

// Variables de contrôle moteur
bool STBY, STEP, DIR, EN, M0, M1, M2;

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
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_EN, OUTPUT);
  pinMode(PIN_M0, OUTPUT);
  pinMode(PIN_M1, OUTPUT);
  pinMode(PIN_M2, OUTPUT);
  pinMode(PIN_FDC_HAUT, INPUT_PULLUP);
  pinMode(PIN_FDC_BAS, INPUT_PULLUP);

  // Appliquer les valeurs initiales
  digitalWrite(PIN_STBY, STBY);
  digitalWrite(PIN_STEP, STEP);
  digitalWrite(PIN_DIR, DIR);
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
// swpulse : nombre de pas, + : avant - : arrière
// microstep : type de pas
// up : mode fdc ou non, 1 : fdc, 0 : pas
int stepper(int swpulse, int microstep, bool up)
{
  if (swpulse > 0)
  {
    DIR = SENS_HAUT;
  }
  else if (swpulse < 0)
  {
    DIR = SENS_BAS;
    swpulse = -swpulse; // Rendre positif pour la boucle
  }
  else
  {
    return 0; // Ne rien faire si swpulse == 0
  }
  if(up) swpulse = 200000;

  // Configurer le microstepping
  M0 = microstep & 0b001;
  M1 = (microstep >> 1) & 0b01;
  M2 = (microstep >> 2) & 0b01;

  EN = 1;
  digitalWrite(PIN_EN, EN);
  digitalWrite(PIN_DIR, DIR);
  digitalWrite(PIN_M0, M0);
  digitalWrite(PIN_M1, M1);
  digitalWrite(PIN_M2, M2);

  for (int i = 0; i < swpulse; i++)
  {
    STEP = 1;
    digitalWrite(PIN_STEP, STEP);
    vTaskDelay(pdMS_TO_TICKS(1));
    STEP = 0;
    digitalWrite(PIN_STEP, STEP);
    vTaskDelay(pdMS_TO_TICKS(4));

    // Arrêt si fin de course détectée
    if ((digitalRead(PIN_FDC_HAUT) == 0) && (up) && (DIR == SENS_HAUT)) // si on monte
    {
      EN = 0;
      digitalWrite(PIN_EN, EN);
      return 1;
    }
    if ((digitalRead(PIN_FDC_BAS) == 0) && (up) && (DIR == SENS_BAS)) // si on descend
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

void display_FDC(void){
  Serial.print("FDC_HAUT : ");
  Serial.print(digitalRead(PIN_FDC_HAUT));
  Serial.print(" | FDC_BAS : ");
  Serial.println(digitalRead(PIN_FDC_BAS));
}
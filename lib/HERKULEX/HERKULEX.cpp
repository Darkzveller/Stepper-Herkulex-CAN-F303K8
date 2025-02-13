#include <Arduino.h>
#include "HardwareSerial.h"
#include <HerkulexServo.h>
// Définition des broches utilisées pour la communication série avec les servos Herkulex
#define PIN_SW_RX PB7 // Broche utilisée pour la réception (RX)
#define PIN_SW_TX PB6 // Broche utilisée pour la transmission (TX)

// Initialisation de la liaison série matérielle sur l'UART1
HardwareSerial Serial1(USART1); // UART1 pour la communication avec les servos Herkulex
// Création du bus de communication pour les servos Herkulex
HerkulexServoBus herkulex_bus(Serial1);
// Création d'un objet servo avec l'adresse de diffusion (HERKULEX_BROADCAST_ID signifie tous les servos connectés)
HerkulexServo my_servo(herkulex_bus, HERKULEX_BROADCAST_ID);
// Variables pour gérer l'intervalle de mise à jour
unsigned long last_update = 0; // Stocke le temps de la dernière mise à jour
unsigned long now = 0;         // Stocke le temps actuel
bool toggle = false;           // Booléen pour alterner entre deux positions
// Variables pour la position du servo
int pos, pos_angle;

void init_serial_1_for_herkulex()
{
  Serial1.setRx(PIN_SW_RX); // Associe la broche RX à l'UART1
  Serial1.setTx(PIN_SW_TX); // Associe la broche TX à l'UART1
  Serial1.begin(115200);    // Initialise la communication série à 115200 bauds
  my_servo.setTorqueOn();   // Active le couple du servo (mise sous tension)
}

void test_herkulex()
{
  herkulex_bus.update(); // Met à jour les servos connectés
  now = millis();        // Récupère le temps actuel en millisecondes

  // Commenté : récupération de la position du servo
  // pos = my_servo.getPosition();
  // pos_angle = (pos-512)*0.325;  // Conversion en degrés
  // Serial.printf("pos : %4d | %4d °\n", pos, pos_angle);

  // Si 5000 ms (5 secondes) se sont écoulées depuis la dernière mise à jour
  if ((now - last_update) > 5000)
  {
    // called every 1000 ms
    if (toggle)
    {
      // Déplace le servo à -90° en 50 cycles, allume la LED verte
      // 512 - 90°/0.325 = 235
      // my_servo.reboot();
      my_servo.setPosition(512 - 0 / 0.325, 50, HerkulexLed::Green);
      // Possibilité d'ajouter d'autres servos avec différentes positions
      // my_servo_2.setPosition(512-10/0.325, 50, HerkulexLed::Blue);
      // my_servo_3.setPosition(512+30/0.325, 50, HerkulexLed::Yellow);
    }
    else
    {
      // Déplace le servo à +45° en 50 cycles, allume la LED bleue
      // 512 + (45° / 0.325) = 650
      // 512 + 90°/0.325 = 789
      my_servo.setPosition(512 + 45 / 0.325, 50, HerkulexLed::Blue);
      my_servo.setTorqueOn();
      // my_servo_2.setPosition(512+10/0.325, 50, HerkulexLed::Blue);
      // my_servo_3.setPosition(512-30/0.325, 50, HerkulexLed::Yellow);
    }
    last_update = now; // Met à jour le dernier temps d'exécution
    toggle = !toggle;  // Alterne entre les deux positions
  }
}

int detect_id(bool activate)
{
  if (activate)
  {
    herkulex_bus.update(); // Met à jour les servos sur le bus

    uint8_t servos_found = 0;
    // Boucle sur tous les ID possibles (0x00 à 0xFD)
    for (uint8_t id = 0; id <= 0xFD; id++)
    {
      HerkulexPacket resp; // Stocke la réponse du servo
      bool success = herkulex_bus.sendPacketAndReadResponse(resp, id, HerkulexCommand::Stat);

      if (success)
      {
        servos_found++; // Incrémente le compteur si un servo est trouvé

        // Affichage de l'ID au format hexadécimal
        if (id <= 0x0F)
        {
          Serial.print("0"); // Ajoute un "0" pour l'alignement des nombres
        }

        Serial.print(id, HEX);
      }
      else
      {
        Serial.print("--"); // Affiche "--" si aucun servo n'est détecté
      }
      // Saut de ligne toutes les 15 adresses affichées
      if (((id + 1) % 0x0F) == 0)
      {
        Serial.println();
      }
      else
      {
        Serial.print(" ");
      }
    }
    // Affichage du nombre total de servos trouvés
    Serial.println();
    Serial.println("Done!");
    Serial.print("Found ");
    Serial.print(servos_found);
    Serial.println(" servos.");

    return 0xFD; // Retourne l'adresse de diffusion (broadcast ID)
  }
  else
  {
    return 0; // Retourne 0 si la détection est désactivée
  }
}

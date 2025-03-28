#ifndef _HERKULEX_H
#define _HERKULEX_H
#include <HardwareSerial.h>
#include <HerkulexServo.h>

#define SERVO_AIMANT_CENTRE  0x04
#define SERVO_AIMANT_GAUCHE  0x03 
#define SERVO_AIMANT_DROIT  0x02
#define SERVO_PIVOT_GAUCHE 0x01
#define SERVO_PIVOT_DROIT 0x05
#define SERVO_PINCE 0x06
#define SERVO_PIVOT_PINCE 0x07

#define RETIRER 0
#define ATTRAPER 1
#define CENTRE 0 // canette cote ramené au centre
#define COTE 1 // canette cote ramené au coté
#define ECARTER 2 // cannette cote écarté pour monter la planche 
#define RETRACTER 0
#define DEPLOYER 1


void init_serial_1_for_herkulex();
void test_herkulex();
int detect_id(bool activate);
void aimant_cote_centre(void);
void aimant_cote_ecarter(void);
void aimant_cote_cote(void);
void cmd_aimant_centre(bool mouvement);
void cmd_aimant_cote(char mouvement);
void test_servo_pivot_gauche(void);
void cmd_pivot_pince(bool mouvement);
void cmd_pince(bool mouvement);


#endif

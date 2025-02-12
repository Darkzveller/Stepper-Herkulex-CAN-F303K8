#ifndef _STEPPER_H
#define _STEPPER_H
#define Caca 0
//* Fonction pour initialiser le moteur pas-à-pas
void initStepper();
void blockStepper(void);

//* Fonction pour la mise en route du moteur pas-à-pas
int stepper(int swpulse, int m0, int m1, int m2, int dir, int dur, bool up );

#endif

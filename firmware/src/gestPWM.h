#ifndef GestPWM_H
#define GestPWM_H
/*--------------------------------------------------------*/
// GestPWM.h
/*--------------------------------------------------------*/
//	Description :	Gestion des PWM 
//			        pour TP1 2016-2017
//
//	Auteur 		: 	C. HUBER
//
//	Version		:	V1.1
//	Compilateur	:	XC32 V1.42 + Harmony 1.08
//
//  Modification : 1.12.2023 SCA : enleve decl. PWMData extern
//
/*--------------------------------------------------------*/

#include <stdint.h>
#include "Mc32DriverAdc.h"


/*--------------------------------------------------------*/
// Définition des fonctions prototypes
/*--------------------------------------------------------*/
#define ANGLEMAX 198 //valeur de décalage à l'origine 
#define OFFSETORIG 99 //valeur maximum absolue de vitesse en % 
#define MAXVALAD 1023 //valeure max de comptage de l'adc 
#define VAL_MAX_TIMER2 1999 //valeure max de comptage du timer2 
#define DIVISION 100//valeur pour récupérer le resultat en % 
#define ANGLE_ABS 180 // valeur d'angle absolue pour le servo 
#define VAL06MS 750 //Nb de tick correspondant à 0,6ms 
#define MAXANGLE 90//angle maximum absolu
#define MAXTICK_TIMER3 2500//plage d'utilisation du timer 2 

typedef struct {
    uint8_t absSpeed;    // vitesse 0 à 99
    uint8_t absAngle;    // Angle  0 à 180
    int8_t SpeedSetting; // consigne vitesse -99 à +99
    int8_t AngleSetting; // consigne angle  -90 à +90
} S_pwmSettings;

extern S_pwmSettings pData; 

void GPWM_Initialize(S_pwmSettings *pData);

// Ces 4 fonctions ont pour paramètre un pointeur sur la structure S_pwmSettings.
void GPWM_GetSettings(S_pwmSettings *pData);	// Obtention vitesse et angle
void GPWM_DispSettings(S_pwmSettings *pData);	// Affichage
void GPWM_ExecPWM(S_pwmSettings *pData);		// Execution PWM et gestion moteur.
void GPWM_ExecPWMSoft(S_pwmSettings *pData);		// Execution PWM software.
int Sweepingmoy(S_ADCResults *AdcRes,int chan); //moyenne glissante

#endif

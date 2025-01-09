/*--------------------------------------------------------*/
// GestPWM.c
/*--------------------------------------------------------*/
//	Description :	Gestion des PWM 
//			        pour TP1 2016-2017
//
//	Auteur 		: 	C. HUBER
//
//	Version		:	V1.1
//	Compilateur	:	XC32 V1.42 + Harmony 1.08
//
/*--------------------------------------------------------*/
 
 
#include "GestPWM.h"
#include "Mc32DriverLcd.h"
#include "bsp.h"
#include "app.h"
#include "Mc32DriverAdc.h"
#include "C:\microchip\harmony\v2_06\framework\peripheral\oc\plib_oc.h"

/***************************************************************
*                                                             *
*                       GPWM_Initialize                       *
*                                                             *
* Description : Cette fonction initialise les champs de la    *
* structure                                                   *
* Paramètres d'entrée : Nom/type                              *
*   -pData : S_pwmSettings                                    *
*                                                             *
*                                                             *
* Paramètre de sortie :                                       *
*   -.                                                        *
*                                                             *
***************************************************************/
void GPWM_Initialize(S_pwmSettings *pData)
{
   // Init les data 
    pData->absSpeed=0;   // vitesse 0 à 99
    pData->absAngle=0;    // Angle  0 à 180
    pData->SpeedSetting=0; // consigne vitesse -99 à +99
    pData->AngleSetting=0;
   // Init état du pont en H
   BSP_EnableHbrige();
    // lance les tmers et OC
   DRV_TMR0_Start();  
   DRV_TMR1_Start();
   DRV_TMR2_Start();
   DRV_TMR3_Start();
   DRV_OC0_Start();
   DRV_OC1_Start();
}
 
/***************************************************************
*                                                             *
*                       GPWM_GetSettings                       *
*                                                             *
* Description : Cette fonction calcul les consignes fct de pot*
* structure                                                   *
* Paramètres d'entrée : Nom/type                              *
*   -pData : S_pwmSettings                                    *
*                                                             *
*                                                             *
* Paramètre de sortie :                                       *
*   -.                                                        *
*                                                             *
***************************************************************/
// Obtention vitesse et angle (mise a jour des 4 champs de la structure)
void GPWM_GetSettings(S_pwmSettings *pData)	
{
    //structure pour résultat de L'AD 
    S_ADCResults AdcRes;
    // Lecture du convertisseur AD
    AdcRes = BSP_ReadAllADC();
    //Calcul du réglage de vittesse  MoyenneADC / 1023 * 198 - 99 
    pData->SpeedSetting = ((abs(((float)Sweepingmoy(&AdcRes,0)/MAXVALAD)*ANGLEMAX))-OFFSETORIG);
    //valeure absolue du calcul plus haut
    pData->absSpeed =abs( pData->SpeedSetting);
    
    //Calcul du réglage de vittesse  MoyenneADC *180/1023
    pData->absAngle = (((float)Sweepingmoy(&AdcRes,1)*ANGLE_ABS)/MAXVALAD);    // Angle  0 à 180
    //réglage de l'angle en prenant compte du signe
    pData->AngleSetting = pData->absAngle-MAXANGLE; // consigne angle  -90 à +90
  
}
/***************************************************************
*                                                             *
*                       Sweepingmoy                           *
*                                                             *
* Description : Cette fonction ser a calculer la moyenne glissante*
* structure                                                   *
* Paramètres d'entrée : Nom/type                              *
*   -AdcRes : S_ADCResults                                    *
*   -chan  : int                                              *
*                                                             *
* Paramètre de sortie : type                                  *
*   -int                                                      *
*                                                             *
***************************************************************/
int Sweepingmoy(S_ADCResults *AdcRes,int chan)
{
    //tableau pour echantillons moyenne glissante
    static int buff1[11] = {0}; 
    static int buff2[11] = {0}; 
    //variable de comtptage pour echantillon
    static int iterator = 0;
    //variable de comtptage pour moyenne
    int i;
    // Stocker la valeur dans le buffer adéquat
    buff2[iterator] = AdcRes->Chan1;
    buff1[iterator] = AdcRes->Chan0;
   
    // Incrémenter l'itérateur et gérer le retour à zéro
    iterator++;
    if (iterator == 10)
    {
        iterator = 0;
     
    }

    // Calculer la moyenne

  
        buff1[10] = 0; // Réinitialiser la somme avant de la recalculer
        buff2[10] = 0;
        for (i = 0; i < 10; i++) 
        {
            buff1[10] += buff1[i]; 
            buff2[10] += buff2[i];
        }
    //selon le canal choisit en entrée renvois la bonne moyenne
    return (chan)? (buff1[10] / 10):(buff2[10] / 10);
}
/***************************************************************
*                                                             *
*                       GPWM_DispSettings                     *
*                                                             *
* Description : Cette fonction fait l'affichage               *
* des information en exploitant la structure                  *
*  structure                                                  *
* Paramètres d'entrée : Nom/type                              *
*   -pData : S_pwmSettings                                    *
* Paramètre de sortie :                                       *
*   -.                                                        *
*                                                             *
***************************************************************/
 

void GPWM_DispSettings(S_pwmSettings *pData)
{
    //déplacement du curseur
    lcd_gotoxy(C1,L2);
    //affiche valeure ded réglage de vitesse 
    printf_lcd("SpeedSetting: %4d", pData->SpeedSetting);
    //vide la ligne
    lcd_ClearLine(L3);
    //déplacement du curseur
    lcd_gotoxy(C1,L3);
    //affiche la vitesse absole 
    printf_lcd("AbsSpeed: %3d",  pData->absSpeed);
    //déplacement du curseur
    lcd_gotoxy(C1,L4);
    //Affiche la vlaeur absolue de l'angle
    printf_lcd("AbsAnlge: %3d",  pData->AngleSetting);
  
    
}
 /***************************************************************
*                                                             *
*                       GPWM_ExecPWM                     *
*                                                             *
* Description : Cette fonction paramèter le pont en H         *
* Fonction de l'angle reglé et  paramètre L'OC                *
*  structure                                                  *
* Paramètres d'entrée : Nom/type                              *
*   -pData : S_pwmSettings                                    *
*                                                             *
* Paramètre de sortie :                                       *
*   -.                                                        *
*                                                             *
***************************************************************/

void GPWM_ExecPWM(S_pwmSettings *pData)
{
    //selon signe de le réglage de vitesse 
    if(pData->SpeedSetting > 0)
    {
        //paramètrage de la direction 
        AIN1_HBRIDGE_W = 1;
        AIN2_HBRIDGE_W = 0;
        STBY_HBRIDGE_W = 1;
    }
    
    else if (pData->SpeedSetting < 0)
    {
        //paramètrage de la direction 
        AIN1_HBRIDGE_W = 0;
        AIN2_HBRIDGE_W = 1;
        STBY_HBRIDGE_W = 1;
    }
    else 
    {
        //met en Standby 
        STBY_HBRIDGE_W = 0; 
    }
    
    //Calcul pour faire correspondre valeure de vitesse en % en Tick pour compteur(timer2) pour moteur DC
    PLIB_OC_PulseWidth16BitSet(OC_ID_2, (pData->absSpeed*VAL_MAX_TIMER2)/DIVISION);
    //Calcul pour faire correspondre valeure de l'angle  en ° en Tick pour compteur(timer3) pour servo
    PLIB_OC_PulseWidth16BitSet(OC_ID_3, (((pData->absAngle)*(MAXTICK_TIMER3/ANGLE_ABS)))+VAL06MS);
}
 /***************************************************************
*                                                             *
*                       GPWM_ExecPWMSoft                      *
*                                                             *
* Description : Cette fonction sert de remplacement à l'OC    *
* elle allume et éteind la led demandée par le CDC            *
*  structure                                                  *
* Paramètres d'entrée : Nom/type                              *
*   -pData : S_pwmSettings                                    *
*                                             *
*                                                             *
* Paramètre de sortie :                                       *
*   -.                                                        *
*                                                             *
***************************************************************/
// Execution PWM software
void GPWM_ExecPWMSoft(S_pwmSettings *pData)
{
    static uint8_t cnt = 0;

    // Dans le cas ou la valeure de vitesse dépasse le cnt 
    if (pData->absSpeed > cnt)
    {
        //éteinnd la led 
        BSP_LEDOff(BSP_LED_2);
    }
    else
    {
        //Sinon l'allume 
        BSP_LEDOn(BSP_LED_2);
    }

    //incrémentation compteur
    cnt++;
    //test valeure max compteur(99))
    if (cnt >= OFFSETORIG)
    {
        //remise à 0
        cnt = 0;
    }
}

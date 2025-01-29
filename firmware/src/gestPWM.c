/*--------------------------------------------------------*/
// GestPWM.c
/*--------------------------------------------------------*/
//	Description :	Gestion des PWM 
//			        pour TP1 2016-2017
//
//	Auteur 		: 	MBR NBN
//
//	Version		:	V1.1
//	Compilateur	:	XC32 V1.42 + Harmony 1.08
//
/*--------------------------------------------------------*/

#include <stdlib.h>         // abs
#include "app.h"
#include "GestPWM.h"
#include "Mc32DriverLcd.h"
#include "bsp.h"
#include "math.h"
#include "peripheral/oc/plib_oc.h"

S_pwmSettings PWMData;      // pour les settings
APP_DATA appData;

// *****************************************************************************
/* Fonction :
    void GPWM_Initialize(S_pwmSettings *pData)
 
  Description :
    Cette fonction initialise les parametre la structure de donn�es	
 
  Param�tres :	
    - pData : Pointe la structure de param�tres PWM (S_pwmSettings).
 
   
*/
// *****************************************************************************
void GPWM_Initialize(S_pwmSettings *pData)
{
    // Init les data
    pData->SpeedSetting = 0;    // SpeedSetting mis � 0
    pData->absSpeed = 0;        // absSpeed mis � 0
    pData->absAngle = 0;        // absAngle mis � 0

    
    // Init �tat du pont en H
    BSP_EnableHbrige();
    
    // lance les timers et OC
    DRV_TMR0_Start();   // Start du timer 1
    DRV_TMR1_Start();   // Start du timer 2
    DRV_TMR2_Start();   // Start du timer 3
    
    
    DRV_OC0_Start();    // Start de l'OC 2
    DRV_OC1_Start();    // Start de l'OC 3
}

// *****************************************************************************
/* Fonction :
    void GPWM_GetSettings(S_pwmSettings *pData)
 
  Description :
    R�cup�re la valeur de vitesse et de l'angle, a l'aide des AD.
    AD(CH0) vitesse
    AD(CH1) angle
    Une moyenne glissante est effectuer sur les valeur lue par l'AD 	
 
  Param�tres :	
    - pData : Pointe la structure de param�tres PWM (S_pwmSettings).
 
   
*/
// *****************************************************************************
void GPWM_GetSettings(S_pwmSettings *pData)	
{
// D�claration des variables statiques pour stocker les valeurs ADC pour la moyenne glissante
static uint16_t valeur_ADC1[TAILLEMOYENNEGLISSANTE] = {0};  // Tableau pour stocker les valeurs de Chan0 (ADC1)
static uint16_t valeur_ADC2[TAILLEMOYENNEGLISSANTE] = {0};  // Tableau pour stocker les valeurs de Chan1 (ADC2)
static uint16_t indexMoyenneGlissante = 0;                  // Indice pour suivre la position dans les tableaux de moyenne glissante
uint8_t indexValeurMoyenneADC;                               // Variable pour it�rer dans les tableaux de moyenne glissante
uint16_t somme1 = 0;                                         // Somme des valeurs de Chan0 pour le calcul de la moyenne
uint16_t somme2 = 0;                                         // Somme des valeurs de Chan1 pour le calcul de la moyenne
uint16_t moyenne1;                                           // Moyenne calcul�e pour Chan0
uint16_t moyenne2;                                           // Moyenne calcul�e pour Chan1

// Lecture des valeurs ADC
appData.AdcRes = BSP_ReadAllADC();

// Stockage des nouvelles valeurs ADC dans les tableaux 
valeur_ADC1[indexMoyenneGlissante] = appData.AdcRes.Chan0;
valeur_ADC2[indexMoyenneGlissante] = appData.AdcRes.Chan1; 

// Mise � jour de l'indice pour la prochaine valeur
indexMoyenneGlissante++;

// Si l'indice d�passe la taille du tableau, il est r�initialis�
if (indexMoyenneGlissante >= MOYENNE_GLISSANTE_PLAGE)  
{
    indexMoyenneGlissante = 0;  // R�initialisation de l'indice
}

// Calcul de la somme de toutes les valeurs dans les tableaux pour les deux canaux ADC
for (indexValeurMoyenneADC = 0; indexValeurMoyenneADC < TAILLEMOYENNEGLISSANTE; indexValeurMoyenneADC++)
{
    somme1 += valeur_ADC1[indexValeurMoyenneADC]; 
    somme2 += valeur_ADC2[indexValeurMoyenneADC];  
}

// Calcul des moyennes des deux canaux ADC
moyenne1 = somme1 / TAILLEMOYENNEGLISSANTE; 
moyenne2 = somme2 / TAILLEMOYENNEGLISSANTE; 

// Calcul du param�tre SpeedSetting bas� sur la moyenne de Chan0
pData->SpeedSetting = (((moyenne1 * VALEURECARTNEGPOS) / ADC_RES) - VALEURMAXVITESSE); 

// Calcul de la vitesse absolue en fonction de SpeedSetting
pData->absSpeed = abs(pData->SpeedSetting);

// Calcul de l'angle en fonction de la moyenne de Chan1 (en degr�s)
pData->absAngle = ((ADC1_Angle_M * moyenne2) / ADC_RES);  

// Calcul de l'angle
//ICI la soustraction de 90 permet de passer la valeur de l'ange de 0 � 180 
pData->AngleSetting = (((ADC1_Angle_M * moyenne2) / ADC_RES) - ADC1_Angle_90);  
}


// *****************************************************************************
/* Fonction :
    void GPWM_DispSettings(S_pwmSettings *pData)
 
  Description :
  Cette fonction permet de controller l'afficheur LCD, pour qu'il afficher les 
  valeur souhait�e
 
  Param�tres :
    - pData : Un pointeur vers la structure de param�tres PWM (S_pwmSettings)
 
 
*/
// *****************************************************************************
void GPWM_DispSettings(S_pwmSettings *pData, int remote)
{
    lcd_putc('\f');
    lcd_gotoxy(1,1);                         // Positionne le curseur � la ligne 1, colonne 1
    if(remote)
    {
        printf_lcd("remote settings");         // Affiche le titre du projet sur l'�cran LCD
    }
    else
    {

        printf_lcd("local settings"); 
    }
    
    //d�placement du curseur
    lcd_gotoxy(C1,L2);
    //affiche valeure ded r�glage de vitesse 
    printf_lcd("SpeedSetting: %4d", pData->SpeedSetting);
    //vide la ligne
    lcd_ClearLine(L3);
    //d�placement du curseur
    lcd_gotoxy(C1,L3);
    //affiche la vitesse absole 
    //printf_lcd("AbsSpeed: %3d",  pData->absSpeed);
    //d�placement du curseur
    //lcd_gotoxy(C1,L4);
    //Affiche la vlaeur absolue de l'angle
    printf_lcd("AbsAnlge: %3d",  pData->AngleSetting);
}

// *****************************************************************************
/* Fonction :
    void GPWM_ExecPWM(S_pwmSettings *pData)
 
  Description :
    Cette fonction permet de gerer les differente sortie n�cessaire pour
    le moteur de l'angle ainsi que pour la vitesse
 
  Param�tres :
- pData : Un pointeur vers la structure de param�tres PWM (S_pwmSettings)
 
*/
// *****************************************************************************
void GPWM_ExecPWM(S_pwmSettings *pData)
{
  //selon signe de le r�glage de vitesse 
    if(pData->SpeedSetting > 0)
    {
        //param�trage de la direction 
        AIN1_HBRIDGE_W = 1;
        AIN2_HBRIDGE_W = 0;
        STBY_HBRIDGE_W = 1;
    }
    else if (pData->SpeedSetting < 0)
    {
        //param�trage de la direction 
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
    //ATTENTIO : le calcul pour la valeur de l'OC se fait avec un angle absolu (de 0 � 180) dans le calcul l'ajout de +90)
    // doit �tre compenser par une soustraction � la reception...  La valeur absolue de l'angle est donn�e entre 0 et 180 et PAS -90 et +90
    PLIB_OC_PulseWidth16BitSet(OC_ID_2, (pData->absSpeed*VAL_MAX_TIMER2)/DIVISION);
    //Calcul pour faire correspondre valeure de l'angle  en � en Tick pour compteur(timer3) pour servo
    PLIB_OC_PulseWidth16BitSet(OC_ID_3, (((  pData->absAngle   )*(MAXTICK_TIMER3/ANGLE_ABS)))+VAL06MS);
}
                      

// Execution PWM software
void GPWM_ExecPWMSoft(S_pwmSettings *pData)
{
    static uint8_t compteur = 0;
 
    // Eteint la led si la vitess et plus grande que le compteur
    if (pData->absSpeed > compteur)
    {
        BSP_LEDOff(BSP_LED_2);
    }
    else
    {
        // Sinon, allumer la LED
        BSP_LEDOn(BSP_LED_2);
    }
 
    // Incr�mente le compteur
    compteur++;
 
    // Remet � z�ro le compteur s'il d�passe la valeur 99
    if (compteur >= CYCLE_CENT_PWM)
    {
        compteur = 0;
    }     
}



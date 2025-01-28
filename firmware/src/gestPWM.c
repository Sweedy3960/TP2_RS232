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
    Cette fonction initialise les parametre la structure de données	
 
  Paramètres :	
    - pData : Pointe la structure de paramètres PWM (S_pwmSettings).
 
   
*/
// *****************************************************************************
void GPWM_Initialize(S_pwmSettings *pData)
{
    // Init les data
    pData->SpeedSetting = 0;    // SpeedSetting mis à 0
    pData->absSpeed = 0;        // absSpeed mis à 0
    pData->absAngle = 0;        // absAngle mis à 0

    
    // Init état du pont en H
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
    Récupère la valeur de vitesse et de l'angle, a l'aide des AD.
    AD(CH0) vitesse
    AD(CH1) angle
    Une moyenne glissante est effectuer sur les valeur lue par l'AD 	
 
  Paramètres :	
    - pData : Pointe la structure de paramètres PWM (S_pwmSettings).
 
   
*/
// *****************************************************************************
void GPWM_GetSettings(S_pwmSettings *pData)	
{
// Déclaration des variables statiques pour stocker les valeurs ADC pour la moyenne glissante
static uint16_t valeur_ADC1[TAILLEMOYENNEGLISSANTE] = {0};  // Tableau pour stocker les valeurs de Chan0 (ADC1)
static uint16_t valeur_ADC2[TAILLEMOYENNEGLISSANTE] = {0};  // Tableau pour stocker les valeurs de Chan1 (ADC2)
static uint16_t indexMoyenneGlissante = 0;                  // Indice pour suivre la position dans les tableaux de moyenne glissante
uint8_t indexValeurMoyenneADC;                               // Variable pour itérer dans les tableaux de moyenne glissante
uint16_t somme1 = 0;                                         // Somme des valeurs de Chan0 pour le calcul de la moyenne
uint16_t somme2 = 0;                                         // Somme des valeurs de Chan1 pour le calcul de la moyenne
uint16_t moyenne1;                                           // Moyenne calculée pour Chan0
uint16_t moyenne2;                                           // Moyenne calculée pour Chan1

// Lecture des valeurs ADC
appData.AdcRes = BSP_ReadAllADC();

// Stockage des nouvelles valeurs ADC dans les tableaux 
valeur_ADC1[indexMoyenneGlissante] = appData.AdcRes.Chan0;
valeur_ADC2[indexMoyenneGlissante] = appData.AdcRes.Chan1; 

// Mise à jour de l'indice pour la prochaine valeur
indexMoyenneGlissante++;

// Si l'indice dépasse la taille du tableau, il est réinitialisé
if (indexMoyenneGlissante >= MOYENNE_GLISSANTE_PLAGE)  
{
    indexMoyenneGlissante = 0;  // Réinitialisation de l'indice
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

// Calcul du paramètre SpeedSetting basé sur la moyenne de Chan0
pData->SpeedSetting = (((moyenne1 * VALEURECARTNEGPOS) / ADC_RES) - VALEURMAXVITESSE); 

// Calcul de la vitesse absolue en fonction de SpeedSetting
pData->absSpeed = abs(pData->SpeedSetting);

// Calcul de l'angle en fonction de la moyenne de Chan1 (en degrés)
pData->absAngle = ((ADC1_Angle_M * moyenne2) / ADC_RES);  

// Calcul de l'angle
pData->AngleSetting = (((ADC1_Angle_M * moyenne2) / ADC_RES) - ADC1_Angle_90);  
}


// *****************************************************************************
/* Fonction :
    void GPWM_DispSettings(S_pwmSettings *pData)
 
  Description :
  Cette fonction permet de controller l'afficheur LCD, pour qu'il afficher les 
  valeur souhaitée
 
  Paramètres :
    - pData : Un pointeur vers la structure de paramètres PWM (S_pwmSettings)
 
 
*/
// *****************************************************************************
void GPWM_DispSettings(S_pwmSettings *pData, int remote)
{
    lcd_putc('\f');
    lcd_gotoxy(1,1);                         // Positionne le curseur à la ligne 1, colonne 1
    if(remote)
    {
        printf_lcd("remote settings");         // Affiche le titre du projet sur l'écran LCD
    }
    else
    {

        printf_lcd("local settings"); 
    }
    
    //déplacement du curseur
    lcd_gotoxy(C1,L2);
    //affiche valeure ded réglage de vitesse 
    printf_lcd("SpeedSetting: %4d", pData->SpeedSetting);
    //vide la ligne
    lcd_ClearLine(L3);
    //déplacement du curseur
    lcd_gotoxy(C1,L3);
    //affiche la vitesse absole 
    //printf_lcd("AbsSpeed: %3d",  pData->absSpeed);
    //déplacement du curseur
    //lcd_gotoxy(C1,L4);
    //Affiche la vlaeur absolue de l'angle
    printf_lcd("AbsAnlge: %3d",  pData->AngleSetting);
}

// *****************************************************************************
/* Fonction :
    void GPWM_ExecPWM(S_pwmSettings *pData)
 
  Description :
    Cette fonction permet de gerer les differente sortie nécessaire pour
    le moteur de l'angle ainsi que pour la vitesse
 
  Paramètres :
- pData : Un pointeur vers la structure de paramètres PWM (S_pwmSettings)
 
*/
// *****************************************************************************
void GPWM_ExecPWM(S_pwmSettings *pData)
{
    
    static int PulseWidthVitesse;
    //static int PulseWidthAngle;
    
    // Gestion sens Moteur et PWM
    
    if(pData->SpeedSetting > 0)
    {
        //paramètrage de la sens horraire
        AIN1_HBRIDGE_W = 1;
        AIN2_HBRIDGE_W = 0;
        STBY_HBRIDGE_W = 1;
    }
    
    else if (pData->SpeedSetting < 0)
    {
        //paramètrage de la sens  anti-horraire
        AIN1_HBRIDGE_W = 0;
        AIN2_HBRIDGE_W = 1;
        STBY_HBRIDGE_W = 1;
    }
    // met le moteur en standby
    else if(pData->SpeedSetting == 0)
    {
        STBY_HBRIDGE_W = 0; 
    }
        
    
    // Calcul de la largeur d'impulsion pour la sortie OC3 en fonction de l'angle
   
    PLIB_OC_PulseWidth16BitSet(OC_ID_3, ((pData->absAngle * PLAGE_OC3 ) / PLAGE_ANGLE_MAX ) + OFFSET_OC3);  //Permet de generer le PWM en choisissant l'OC et la largeur d'impulsion
    
    // Calcul de la largeur d'impulsion pour la sortie OC2 en fonction de l'angle
    PulseWidthVitesse = ((pData->absSpeed * DRV_TMR1_PeriodValueGet())/99);
    PLIB_OC_PulseWidth16BitSet(OC_ID_2,PulseWidthVitesse);        ///Permet de generer le PWM en choisissant l'OC et la largeur d'impulsion
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
 
    // Incrémente le compteur
    compteur++;
 
    // Remet à zéro le compteur s'il dépasse la valeur 99
    if (compteur >= CYCLE_CENT_PWM)
    {
        compteur = 0;
    }     
}



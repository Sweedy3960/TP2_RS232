/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include "Mc32DriverLcd.h"
#include "bsp.h"
#include "gestPWM.h"
#include "Mc32gest_RS232.h"
#include "Mc32CalCrc16.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
 */
S_pwmSettings PWMData;
S_pwmSettings PWMDataToSend;
APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
 */

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
 */


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize(void) {
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;


    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}

/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks(void) {

    /* Check the application's current state. */
    switch (appData.state) {
            /* Application's initial state. */
        case APP_STATE_INIT:
        {
            // Initialisation
            initialisation();
            InitFifoComm();
            appData.state = APP_STATE_WAIT;
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
            static int CommStatus =0;
            static int cnt=0;
            static int cntMessageEronous;
            static int cyclFlag;
            CommStatus = GetMessage(&PWMData);
            // R�ception param. remote  c'est ca qui v pas 
            /*
            if(CommStatus == 0)
            {
                cntMessageEronous++;
                if(cntMessageEronous >= 10)
                {
                    cyclFlag =0;
                    cntMessageEronous=0;
                }
            }
            else
            {
                cyclFlag =1;
            }
                   */
            
            if (CommStatus == 0) // local ?
            {
                    GPWM_GetSettings(&PWMData); // local
            }
            else
            {
                    GPWM_GetSettings(&PWMDataToSend); // remote
                     
            }
           
            
             
            GPWM_DispSettings(&PWMData, CommStatus);
            GPWM_ExecPWM(&PWMData);
            
            
            if (cnt == 5)
            {
                cnt = 0;
                // Envoi valeurs 
                if (CommStatus == 0) // local ?
                {
                   SendMessage(&PWMData); // local
                  
                   
                }
                else
                {
                    SendMessage(&PWMDataToSend); // remote
                     
                }
                appData.state = APP_STATE_WAIT; 
                
                
            }
            else
            {
                cnt++;
            }
            
            appData.state = APP_STATE_WAIT;
            break;

            // Lecture pot. 
            
         
        }

        case APP_STATE_WAIT:
        {
            break;
        }

            /* TODO: implement your application state machine.*/


            /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}
/* Fonction :
    void APP_Timer1CallBack(void)
 
  Description :
	Fonction de callback pour le timer 1
    avec un compteur afin de laisser l'�cran d'init au lancement de 3 secondes
  Param�tres :	
    -
*/

void APP_Timer1CallBack(void) {

    static uint8_t CntInit = 0;
    // compteur qui gere le temps d'init
    CntInit++;
    if (CntInit >= TEMP_INIT) 
    {
        
        CntInit = 150;
        APP_UpdateState(APP_STATE_SERVICE_TASKS);
    }


}

void APP_Timer2CallBack(void) 
{
    
}

void APP_Timer3CallBack(void) 
{
    
}

void APP_Timer4CallBack(void) 
{

    GPWM_ExecPWMSoft(&PWMData);
}

void APP_UpdateState(APP_STATES newState) {
    appData.state = newState;
}

/* Fonction :
    void Initialisation(void)
 
  Description :
    permet d'initialiser le lcd, l'adc. Affiche les lignes initiales sur le LCD
	et appel la fonction GPWM_Initialize(&PWMData);
 
  Param�tres :	
    -
*/

void initialisation(void) 
{
    lcd_init(); //initialise le LCD
    printf_lcd("TP1 PWM 2024-2025"); //affiche sur le LCD
    lcd_gotoxy(1, 2); //va sur la deuxieme ligne de l'ecran LCD
    printf_lcd("Clauzel Aymeric "); //affiche sur le LCD
    lcd_gotoxy(1, 3); //va sur la deuxieme ligne de l'ecran LCD
    printf_lcd("Bucher Mathieu"); //affiche sur le LCD
    lcd_bl_on(); //allume les Backlight
    BSP_InitADC10(); //initialisae l'adc
    LEDOff(); //�teint toutes les leds
    GPWM_Initialize(&PWMData);
    DRV_USART0_Initialize();
}

/* Fonction :
    void LEDOff(void)
 
  Description :
    Cette fonction �teints les leds 0 � 7
 
  Param�tres :	
    -
*/

void LEDOff(void) {
    //Eteints les leds 0 à 7
    BSP_LEDOff(BSP_LED_0);
    BSP_LEDOff(BSP_LED_1);
    BSP_LEDOff(BSP_LED_2);
    BSP_LEDOff(BSP_LED_3);
    BSP_LEDOff(BSP_LED_4);
    BSP_LEDOff(BSP_LED_5);
    BSP_LEDOff(BSP_LED_6);
    BSP_LEDOff(BSP_LED_7);
}


/*******************************************************************************
 End of File
 */

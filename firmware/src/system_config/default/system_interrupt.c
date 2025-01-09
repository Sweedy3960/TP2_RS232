/*******************************************************************************
 System Interrupts File

  File Name:
    system_interrupt.c

  Summary:
    Raw ISR definitions.

  Description:
    This file contains a definitions of the raw ISRs required to support the
    interrupt sub-system.

  Summary:
    This file contains source code for the interrupt vector functions in the
    system.

  Description:
    This file contains source code for the interrupt vector functions in the
    system.  It implements the system and part specific vector "stub" functions
    from which the individual "Tasks" functions are called for any modules
    executing interrupt-driven in the MPLAB Harmony system.

  Remarks:
    This file requires access to the systemObjects global data structure that
    contains the object handles to all MPLAB Harmony module objects executing
    interrupt-driven in the system.  These handles are passed into the individual
    module "Tasks" functions to identify the instance of the module to maintain.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2011-2014 released Microchip Technology Inc.  All rights reserved.

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

#include "system/common/sys_common.h"
#include "app.h"
#include "system_definitions.h"
#include "gestPWM.h"
#include "bsp.h"
// *****************************************************************************
// *****************************************************************************
// Section: System Interrupt Vector Functions
// *****************************************************************************
// *****************************************************************************
S_pwmSettings pData;  

void __ISR(_TIMER_1_VECTOR, ipl4AUTO) IntHandlerDrvTmrInstance0(void)
{
    static uint8_t counter3sec;
    //allume led 0 pour mesure 
    BSP_LEDOn(BSP_LED_0);
    //si compteur d�passe les 3 secondes 
    if (counter3sec > _3STIMER)
    {   
        //appels des fonction demand�s par le cdc 
        GPWM_GetSettings(&pData);
        GPWM_DispSettings(&pData);
        GPWM_ExecPWM(&pData);
        //r�initialise le compteur
        counter3sec = (_3STIMER-1); 
    }
    else
    {
        //incr�ment
        counter3sec ++;
    }
    //remet � 0 le flag pour lib�re
    PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_TIMER_1);
    //�teidn la led
    BSP_LEDOff(BSP_LED_0); 
   
}
void __ISR(_TIMER_2_VECTOR, ipl0AUTO) IntHandlerDrvTmrInstance1(void)
{
    //remet � 0 le flag
    PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_TIMER_2);
}
void __ISR(_TIMER_3_VECTOR, ipl0AUTO) IntHandlerDrvTmrInstance2(void)
{
    //remet � 0 le flag
    PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_TIMER_3);
}
void __ISR(_TIMER_4_VECTOR, ipl4AUTO) IntHandlerDrvTmrInstance3(void)
{
    //allume la LED 1 pour mesures
    BSP_LEDOn(BSP_LED_1);

    // Ex�cute la PWM logiciel
    GPWM_ExecPWMSoft(&pData);

    // �teint la LED BSP_LED_1 apr�s la PWM logiciel
    BSP_LEDOff(BSP_LED_1);

    //remet � 0 le flag
    PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_TIMER_4);
}
 
 /*******************************************************************************
 End of File
*/

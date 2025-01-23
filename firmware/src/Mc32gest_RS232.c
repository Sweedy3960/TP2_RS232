// Mc32Gest_RS232.C
// Canevas manipulatio TP2 RS232 SLO2 2017-18
// Fonctions d'�mission et de r�ception des message
// CHR 20.12.2016 ajout traitement int error
// CHR 22.12.2016 evolution des marquers observation int Usart
// SCA 03.01.2018 nettoy� r�ponse interrupt pour ne laisser que les 3 ifs

#include <xc.h>
#include <sys/attribs.h>
#include "system_definitions.h"
#include "bsp.h"
#include <GenericTypeDefs.h>
#include "app.h"
#include "GesFifoTh32.h"
#include "Mc32gest_RS232.h"
#include "gestPWM.h"
#include "Mc32CalCrc16.h"
#define TAILLE_TABLEAU 3

typedef union {
        uint16_t val;
        struct {uint8_t lsb;
                uint8_t msb;} shl;
} U_manip16;


// Definition pour les messages
#define MESS_SIZE  5
// avec int8_t besoin -86 au lieu de 0xAA
#define STX_code  (-86)

// Structure d�crivant le message
typedef struct {
    uint8_t Start;
    int8_t  Speed;
    int8_t  Angle;
    uint8_t MsbCrc;
    uint8_t LsbCrc;
} StruMess;


// Struct pour �mission des messages
StruMess TxMess;
// Struct pour r�ception des messages
StruMess RxMess;

// Declaration des FIFO pour r�ception et �mission
#define FIFO_RX_SIZE ( (4*MESS_SIZE) + 1)  // 4 messages
#define FIFO_TX_SIZE ( (4*MESS_SIZE) + 1)  // 4 messages

int8_t fifoRX[FIFO_RX_SIZE];
// Declaration du descripteur du FIFO de r�ception
S_fifo descrFifoRX;


int8_t fifoTX[FIFO_TX_SIZE];
// Declaration du descripteur du FIFO d'�mission
S_fifo descrFifoTX;


// Initialisation de la communication s�rielle
void InitFifoComm(void)
{    
    // Initialisation du fifo de r�ception
    InitFifo ( &descrFifoRX, FIFO_RX_SIZE, fifoRX, 0 );
    // Initialisation du fifo d'�mission
    InitFifo ( &descrFifoTX, FIFO_TX_SIZE, fifoTX, 0 );
    
    // Init RTS 
    RS232_RTS = 1;   // interdit �mission par l'autre
   
} // InitComm

 
// Valeur de retour 0  = pas de message re�u donc local (data non modifi�)
// Valeur de retour 1  = message re�u donc en remote (data mis � jour)
int GetMessage(S_pwmSettings *pData)
{
    int commStatus = 0;
    static uint8_t Decalage = 0, DecalageMessage = 0;
    int8_t i = 0, j = 0; 
    uint16_t ValCrc16 = 0xFFFF;
    int32_t NbCharToRead;
    // Traitement de r�ception � introduire ICI
    // Lecture et d�codage fifo r�ception
    // ...
    GetCharFromFifo(&descrFifoRX, &i); // &monFifoRX[0]]
    fifoRX[Decalage] = i; 
    if (Decalage > 20)
        Decalage = 0; 
    else 
        Decalage++; 
    // Message pr�sent dans le FIFO?
    NbCharToRead = GetReadSize(&descrFifoRX);
    // Si >= taille message alors traite
    if (NbCharToRead >= MESS_SIZE)
    {
        //commStatus = 1;
        // Analyse du contenu du message
        //test
        if ((fifoRX[0] == 0xAA) && (fifoRX[5] == 0xAA))
        {
            // Calcul du Crc pour v�rification du message
            ValCrc16 = updateCRC16(ValCrc16, 0xAA);
            ValCrc16 = updateCRC16(ValCrc16, fifoRX[1]);
            ValCrc16 = updateCRC16(ValCrc16, fifoRX[2]);
            // Comparaison du Crc calcule avec le Crc recu
            if (ValCrc16 == ((fifoRX[3] << 8) + fifoRX[4]))
            {
                // Mise en memoire des valeurs de vitesse et d'angle recues
                pData->AngleSetting = fifoRX[1];
                pData->SpeedSetting = fifoRX[2];
                pData->absSpeed = abs(fifoRX[2]);
                // Le message etant complet, on peut le nettoyer de la fifo
                DecalageMessage = 5;
                // On renvoie qu'un message a �t� recu
                commStatus = 1;
            }
        }
        else
        {
            // Le message n'�tant pas bon, on nettoie juste le byte le plus vieux de la fifo
            DecalageMessage = 1;
        }
        // Decalage du tableau intermediaire contenant le fifo selon les bytes deja lus ou utilises
        for (j = 0; j <= NbCharToRead; j++)
        {
            fifoRX[j] = fifoRX[DecalageMessage + j];
        }
    }
    // Gestion controle de flux de la r�ception
    if(GetWriteSpace ( &descrFifoRX) >= (2*MESS_SIZE)) 
    {
        // autorise �mission par l'autre
        RS232_RTS = 0;
    }
    return commStatus;
} // GetMessage

// Fonction d'envoi des messages, appel cyclique
void SendMessage(S_pwmSettings *pData)
{
    int8_t freeSize;
    uint16_t ValCrc16 = 0xFFFF;
    // Traitement �mission � introduire ICI
    // Formatage message et remplissage fifo �mission
    // ...
    // Test si place Pour �crire 1 message 
    freeSize = GetWriteSpace (&descrFifoTX);
    if (freeSize >= MESS_SIZE ) 
    {
        // Compose le message
        TxMess.Start = 0xAA;
        ValCrc16 = updateCRC16(ValCrc16,TxMess.Start  );
        TxMess.Speed= pData->SpeedSetting;
        ValCrc16 = updateCRC16(ValCrc16,TxMess.Speed);
        TxMess.Angle= pData->AngleSetting;
        ValCrc16 = updateCRC16(ValCrc16, TxMess.Angle);
        TxMess.LsbCrc = (ValCrc16 & 0x00FF);
        TxMess.MsbCrc = (ValCrc16 & 0xFF00)>>8;
        // D�pose le message dans le fifo
        PutCharInFifo ( &descrFifoTX, 0xAA);
        PutCharInFifo ( &descrFifoTX, TxMess.Speed);
        PutCharInFifo ( &descrFifoTX, TxMess.Angle);
        PutCharInFifo ( &descrFifoTX, TxMess.MsbCrc);
        PutCharInFifo ( &descrFifoTX, TxMess.LsbCrc);
    }
    
   
    
    
    // Gestion du controle de flux
    // si on a un caract�re � envoyer et que CTS = 0
    freeSize = GetReadSize(&descrFifoTX);
    if ((RS232_CTS == 0) && (freeSize > 0))
    {
        // Autorise int �mission    
        PLIB_INT_SourceEnable(INT_ID_0, INT_SOURCE_USART_1_TRANSMIT);                
    }
}


// Interruption USART1
// !!!!!!!!
// Attention ne pas oublier de supprimer la r�ponse g�n�r�e dans system_interrupt
// !!!!!!!!
 void __ISR(_UART_1_VECTOR, ipl5AUTO)_IntHandlerDrvUsartInstance0()
{
    USART_ERROR UsartStatus;    
    uint8_t freeSize, TXsize;
    int8_t c;
    int8_t i_cts = 0;
    BOOL TxBuffFull;


    // Marque d�but interruption avec Led3
    LED3_W = 1;
    
    // Is this an Error interrupt ?
    if (PLIB_INT_SourceFlagGet(INT_ID_0, INT_SOURCE_USART_1_ERROR) &&
            PLIB_INT_SourceIsEnabled(INT_ID_0, INT_SOURCE_USART_1_ERROR)) 
    {
        /* Clear pending interrupt */
        PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_USART_1_ERROR);
        // Traitement de l'erreur � la r�ception.
    }
   

    // Is this an RX interrupt ?
    if ( PLIB_INT_SourceFlagGet(INT_ID_0, INT_SOURCE_USART_1_RECEIVE) &&
                 PLIB_INT_SourceIsEnabled(INT_ID_0, INT_SOURCE_USART_1_RECEIVE) ) 
    {

        // Oui Test si erreur parit� ou overrun
        UsartStatus = PLIB_USART_ErrorsGet(USART_ID_1);

        if ( (UsartStatus & (USART_ERROR_PARITY |
                             USART_ERROR_FRAMING | USART_ERROR_RECEIVER_OVERRUN)) == 0)
        {

            // Traitement RX � faire ICI
            // Lecture des caract�res depuis le buffer HW -> fifo SW
			//  (pour savoir s'il y a une data dans le buffer HW RX : PLIB_USART_ReceiverDataIsAvailable())
			//  (Lecture via fonction PLIB_USART_ReceiverByteReceive())
            // ...
            // transfert dans le FIFO software
            // de tous les char re�us
            while (PLIB_USART_ReceiverDataIsAvailable(USART_ID_1))
            
               
            {
                c = PLIB_USART_ReceiverByteReceive(USART_ID_1);
                PutCharInFifo ( &descrFifoRX,c);
            }

            LED4_W = !LED4_R; // Toggle Led4
            // buffer is empty, clear interrupt flag
            PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_USART_1_RECEIVE);
        }
        else
        {
            // Suppression des erreurs
            // La lecture des erreurs les efface
            // sauf pour overrun
            if ((UsartStatus & USART_ERROR_RECEIVER_OVERRUN) == USART_ERROR_RECEIVER_OVERRUN) 
            {
                PLIB_USART_ReceiverOverrunErrorClear(USART_ID_1);
            }
        }
        freeSize = GetWriteSpace ( &descrFifoRX);
        // A cause du cas un int pour 6 char � de 8 = 6
        if (freeSize <= 6 ) 
        {
            // Contr�le de flux : demande stop �mission
            RS232_RTS = 1;
        }
    }    
                         
            
            

        
        
     // end if RX

    
    // Is this an TX interrupt ?
    if ( PLIB_INT_SourceFlagGet(INT_ID_0, INT_SOURCE_USART_1_TRANSMIT) &&
                 PLIB_INT_SourceIsEnabled(INT_ID_0, INT_SOURCE_USART_1_TRANSMIT) ) 
    {

        // Traitement TX � faire ICI
        // Envoi des caract�res depuis le fifo SW -> buffer HW
            
        // Avant d'�mettre, on v�rifie 3 conditions :
        //  Si CTS = 0 autorisation d'�mettre (entr�e RS232_CTS)
        //  S'il y a un carat�res � �mettre dans le fifo
        //  S'il y a de la place dans le buffer d'�mission (PLIB_USART_TransmitterBufferIsFull)
        //   (envoi avec PLIB_USART_TransmitterByteSend())
       
        // ...
        TXsize = GetReadSize (&descrFifoTX);
 // i_cts = input(RS232_CTS);
 // On v�rifie 3 conditions :
 // Si CTS = 0 (autorisation d'�mettre)
 // Si il y a un caract�re � �mettre
 // Si le txreg est bien disponible
        i_cts = RS232_CTS;
 // Il est possible de d�poser un caract�re
 // tant que le tampon n'est pas plein
        TxBuffFull = PLIB_USART_TransmitterBufferIsFull(USART_ID_1);
        if ( (i_cts == 0) && ( TXsize > 0 ) &&
        TxBuffFull == false ) 
        {
            do 
            {
                GetCharFromFifo(&descrFifoTX, &c);
                PLIB_USART_TransmitterByteSend(USART_ID_1, c);
 
                i_cts = RS232_CTS;
                TXsize = GetReadSize (&descrFifoTX);
                TxBuffFull =PLIB_USART_TransmitterBufferIsFull (USART_ID_1);

 
            }while ( (i_cts == 0) && ( TXsize > 0 ) && TxBuffFull==false );
 
            LED5_W = !LED5_R; // Toggle Led5
            // Clear the TX interrupt Flag
            // (Seulement apr�s TX)
            PLIB_INT_SourceFlagClear(INT_ID_0,
            INT_SOURCE_USART_1_TRANSMIT);
            if (TXsize == 0) 
            {
            // disable TX interrupt 
            //(pour �viter une int inutile)
                PLIB_INT_SourceDisable(INT_ID_0,INT_SOURCE_USART_1_TRANSMIT);
 
            }
        } 
        else
        {
 // disable TX interrupt
            PLIB_INT_SourceDisable(INT_ID_0,INT_SOURCE_USART_1_TRANSMIT);
         }
    }
    // Marque fin interruption avec Led3
    LED3_W = 0;
} // end __ISR Usart 1





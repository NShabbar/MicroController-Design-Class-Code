/* 
 * File:   Protocol2.c
 * Author: Nadia
 *
 * Created on February 7, 2023, 11:50 AM
 */

#include "Uart.h"
#include "BOARD.h"
#include <xc.h>
#include <math.h>
#include <sys/attribs.h>
#include "CircularBuffer.h"
#include "Uart.h"
#include "Protocol2.h"

#define END \r\n

typedef struct rxpADT {
    uint8_t ID;      
    uint8_t len;
    uint8_t checkSum; 
    unsigned char payLoad[MAXPAYLOADLENGTH];
  }rxpADT; 

  typedef struct rxpBuffObj {
    int head; // location in circ buffer
    int tail;
    rxp *buffer;
} rxpBuffObj;

enum State{
    STATE_START,
    STATE_WAITING_LEN,
    STATE_HEADER,
    STATE_WAITING_ID,
    STATE_ID,
    STATE_WAITING_TAIL,
    STATE_CHECKSUM,
    STATE_WAITING_END,
    STATE_END
};
  /**
 * @Function Protocol_Init(baudrate)
 * @param Legal Uart baudrate
 * @return SUCCESS (true) or ERROR (false)
 * @brief Initializes Uart1 for stream I/O to the lab PC via a USB 
 *        virtual comm port. Baudrate must be a legal value. 
 * @author instructor W2022 */
int Protocol_Init(unsigned long baudrate);
/**
 * @Function unsigned char Protocol_QueuePacket()
 * @param none
 * @return the buffer full flag: 1 if full
 * @brief Place in the main event loop (or in a timer) to continually check 
 *        for completed incoming packets and then queue them into 
 *        the RX circular buffer. The buffer's size is set by constant
 *        PACKETBUFFERSIZE.
 * @author instructor W2023 */
uint8_t Protocol_QueuePacket ();
/**
 * @Function int Protocol_GetInPacket(uint8_t *type, uint8_t *len, uchar *msg)
 * @param *type, *len, *msg
 * @return SUCCESS (true) or WAITING (false)
 * @brief Reads the next packet from the packet Buffer 
 * @author instructor W2022 */
int Protocol_GetInPacket(uint8_t *type, uint8_t *len, unsigned char *msg);
/**
 * @Function int Protocol_SendDebugMessage(char *Message)
 * @param Message, Proper C string to send out
 * @return SUCCESS (true) or ERROR (false)
 * @brief Takes in a proper C-formatted string and sends it out using ID_DEBUG
 * @warning this takes an array, do <b>NOT</b> call sprintf as an argument.
 * @author mdunne */
int Protocol_SendDebugMessage(char *Message);
/**
 * @Function int Protocol_SendPacket(unsigned char len, void *Payload)
 * @param len, length of full <b>Payload</b> variable
 * @param Payload, pointer to data
 * @return SUCCESS (true) or ERROR (false)
 * @brief composes and sends a full packet
 * @author instructor W2022 */
int Protocol_SendPacket(unsigned char len, unsigned char ID, void *Payload);
/**
 @Function unsigned char Protocol_ReadNextID(void)
 * @param None
 * @return Reads the ID of the next available Packet
 * @brief Returns ID or 0 if no packets are available
 * @author instructor W2022 */
unsigned char Protocol_ReadNextPacketID(void);
/**
 * @Function flushPacketBuffer()
 * @param none
 * @return none
 * @brief flushes the rx packet circular buffer  
 * @author instructor W2022 */
void flushPacketBuffer ();

unsigned int convertEndian(unsigned int *);
/*******************************************************************************
 * PRIVATE FUNCTIONS
 * Generally these functions would not be exposed but due to the learning nature 
 * of the class some are are noted to help you organize the code internal 
 * to the module. 
 ******************************************************************************/
/* BuildRxPacket() should implement a state machine to build an incoming
 * packet incrementally and return it completed in the called argument packet
 * structure (rxPacket is a pointer to a packet struct). The state machine should
 * progress through discrete states as each incoming byte is processed.
 * 
 * To help you get started, the following ADT is an example of a structure 
 * intended to contain a single rx packet. 
 * typedef struct rxpT {
 *    uint8_t ID;      
 *    uint8_t len;
 *    uint8_t checkSum; 
 *    unsigned char payLoad[MAXPAYLOADLENGTH];
 * }*rxpADT; 
 *   rxpADT rxPacket ...
 * Now consider how to create another structure for use as a circular buffer
 * containing a PACKETBUFFERSIZE number of these rxpT packet structures.
 ******************************************************************************/
uint8_t BuildRxPacket (rxp rxPacket, unsigned char reset){
    static State state = STATE_START;
    while (state != STATE_END){
        switch(state){
            case STATE_START:
                break;
            case STATE_WAITING_LEN:
                break;
            case STATE_HEADER:
                break;
            case STATE_WAITING_ID:
                break;
            case STATE_ID:
                break;
            case STATE_WAITING_TAIL:
                break;
            case STATE_CHECKSUM:
                break;
            case STATE_WAITING_END:
                break;
            case STATE_END:
                break;
        }
    }
}

/**
 * @Function char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned 
char curChecksum)
 * @param charIn, new char to add to the checksum
 * @param curChecksum, current checksum, most likely the last return of this 
function, can use 0 to reset
 * @return the new checksum value
 * @brief Returns the BSD checksum of the char stream given the curChecksum and the
new char
 * @author mdunne */
unsigned char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned char 
curChecksum);
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



typedef struct rxpT {
    uint8_t ID;
    uint8_t len;
    uint8_t checkSum;
    unsigned char *payLoad;
} *rxpADT;

typedef struct rxpBuffObj {
    int head; // location in circ buffer
    int tail;
    rxpADT *buffer;
} rxpBuffObj;

typedef enum PacketState {
    STATE_HEAD,
    STATE_LEN,
    STATE_ID,
    STATE_PAYL,
    STATE_TAIL,
    STATE_CHECKSUM,
    STATE_END_R,
    STATE_END_N
} PacketState;

static PacketState state;

//Private Helper Functions
uint8_t BuildRxPacket(rxpADT rxPacket, unsigned char byte);
unsigned char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned char
        curChecksum);

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
uint8_t Protocol_QueuePacket();
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
void flushPacketBuffer();

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
uint8_t BuildRxPacket(rxpADT rxPacket, unsigned char byte) {
    switch (state) {
        case STATE_HEAD:
            if (byte == HEAD) {
                rxPacket-> checkSum = 0;
                for (int i = 0; i <= MAXPAYLOADLENGTH; i++) {
                    rxPacket -> payLoad[i] = 0;
                }
                state = STATE_LEN;
                break;
            } else {
                state = STATE_HEAD; // fill in errors here.
                break;
            }
            break;
        case STATE_LEN:
            if (byte <= MAXPAYLOADLENGTH) {
                rxPacket -> len = byte;
                state = STATE_ID;
                break;
            } else {
                state = STATE_HEAD;
                break;
            }
            break;
        case STATE_ID:
            if (byte) {
                rxPacket -> ID = byte;
                rxPacket -> checkSum = Protocol_CalcIterativeChecksum(rxPacket-> checkSum, byte);
                state = STATE_PAYL;
                break;
            } else {
                state = STATE_HEAD; // fill in errors here.
                break;
            }
            break;
        case STATE_PAYL:;
            int count = 1;
            if (byte == TAIL) {
                state = STATE_HEAD; // call error here.
                break;
            }
            rxPacket -> checkSum = Protocol_CalcIterativeChecksum(rxPacket -> checkSum, byte);
            rxPacket -> payLoad[count] = byte;
            count++;
            if (count == ((rxPacket -> len))) {
                state = STATE_TAIL;
                break;
            }
            break;
        case STATE_TAIL:
            if (byte == TAIL) {
                state = STATE_CHECKSUM;
            } else {
                state = STATE_HEAD; // fill in errors here.
                break;
            }
            break;
        case STATE_CHECKSUM:
            if (byte == rxPacket -> checkSum){
                state = STATE_END_R;
                break;
            }else{
                state = STATE_HEAD; // fill in errors here.
                break;
            }
            break;
        case STATE_END_R:
            if (byte == '\r'){
                state = STATE_END_N;
                break;
            }else{
                state = STATE_HEAD; // fill in errors here.
                break;
            }
            break;
        case STATE_END_N:
            if (byte == '\n'){
                state = STATE_HEAD;
                break;
            }else{
                state = STATE_HEAD; // fill in errors here.
                break;
            }
            break;
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
        curChecksum){
    curChecksum = (curChecksum >> 1) + (curChecksum << 7);
    curChecksum += charIn;
    return curChecksum;
}

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
#include<assert.h>
#include <sys/attribs.h>
#include "CircularBuffer.h"
#include "Uart.h"
#include "MessageIDs.h"
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
} *rxpBuffObj;

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
static rxpBuffObj RX;
static rxpADT rxPacket;

/*******************************************************************************
 * PRIVATE FUNCTIONS
 * Generally these functions would not be exposed but due to the learning nature 
 * of the class some are are noted to help you organize the code internal 
 * to the module. 
 ******************************************************************************/
uint8_t BuildRxPacket(rxpADT rxPacket, unsigned char byte);
unsigned char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned char curChecksum);
rxpBuffObj RXBuffer_init();
void freeRXBuffer(rxpBuffObj *pRX);
int RX_isFull(rxpBuffObj RX);
int RX_isEmpty(rxpBuffObj RX);
int WritetoRX(rxpBuffObj RX, rxpADT data);
rxpADT ReadfromRX(rxpBuffObj RX);
void freeRXPacket(rxpADT *pRX);
rxpADT newPacket();
/*******************************************************************************
 ******************************************************************************/

/**
 * @Function Protocol_Init(baudrate)
 * @param Legal Uart baudrate
 * @return SUCCESS (true) or ERROR (false)
 * @brief Initializes Uart1 for stream I/O to the lab PC via a USB 
 *        virtual comm port. Baudrate must be a legal value. 
 * @author instructor W2022 */
int Protocol_Init(unsigned long baudrate){
    Uart_Init(baudrate);
    RX = RXBuffer_init();
    rxPacket = NULL;
    if (RX == NULL){
        return 0;
    }
    state = STATE_HEAD;
    return 1;
}

/**
 * @Function unsigned char Protocol_QueuePacket()
 * @param none
 * @return the buffer full flag: 1 if full
 * @brief Place in the main event loop (or in a timer) to continually check 
 *        for completed incoming packets and then queue them into 
 *        the RX circular buffer. The buffer's size is set by constant
 *        PACKETBUFFERSIZE.
 * @author instructor W2023 */
uint8_t Protocol_QueuePacket() {
    if (RX_isEmpty(RX)) {
        return 0; // make an error for this to return
    }
    if (RX_isFull(RX)){
        return 1;
    }
    if (rxPacket == NULL){
        rxPacket = newPacket();
    }
    while (!RX_isEmpty(RX)) {
        BuildRxPacket(rxPacket, GetChar());
        if (state = STATE_END_N) {
            if (rxPacket -> ID == ID_LEDS_GET) {
                unsigned char payl = LEDS_GET();
                Protocol_SendPacket(2, ID_LEDS_STATE, &payl);
                freeRXPacket(&rxPacket);
            }
            if (rxPacket -> ID == ID_LEDS_SET) {
                LEDS_SET(rxPacket -> payLoad[0]);
                freeRXPacket(&rxPacket);
            }
            WritetoRX(RX, rxPacket);
            rxPacket = NULL;
        }
    }
}
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
int Protocol_SendPacket(unsigned char len, unsigned char ID, unsigned char *Payload) {
    unsigned char data = PutChar(HEAD);
    if (data == false){
        return 0; // create an error to return here.
    }
    data = PutChar(len);
    if (data == false){
        return 0; // create an error to return here.
    }
    data = PutChar(ID);
    uint8_t checks = 0;
    checks = Protocol_CalcIterativeChecksum(checks, data);
    if (data == false){
        return 0; // create an error to return here.
    }
    for (int i = 1; i <= len; i++) {
        data = PutChar(Payload[i]);
        checks = Protocol_CalcIterativeChecksum(checks, data);
        if (data == false){
            return 0; // create an error to return here.
        }
    }
    data = PutChar(TAIL);
    if (data == false){
        return 0; // create an error to return here.
    }
    data = PutChar(checks);
    if (data == false){
        return 0; // create an error to return here.
    }
    data = PutChar('\r');
    if (data == false){
        return 0; // create an error to return here.
    }
    data = PutChar('\n');
    if (data == false){
        return 0; // create an error to return here.
    }
}
/**
 @Function unsigned char Protocol_ReadNextID(void)
 * @param None
 * @return Reads the ID of the next available Packet
 * @brief Returns ID or 0 if no packets are available
 * @author instructor W2022 */
unsigned char Protocol_ReadNextPacketID(void){
    rxpADT packet = RX -> buffer[RX -> head];
    if (packet == NULL){
        return 0;
    }
    return packet -> ID;
}
/**
 * @Function flushPacketBuffer()
 * @param none
 * @return none
 * @brief flushes the rx packet circular buffer  
 * @author instructor W2022 */
void flushPacketBuffer(){
    for (int i = 0; i <= PACKETBUFFERSIZE; i++){
         freeRXPacket(&(RX -> buffer[i]));
    }
    RX -> head = 0;
    RX -> tail = 0;
}

unsigned int convertEndian(unsigned int* data){
    unsigned int result;
    //result = byte0 | byte1 | byte2 | byte3;
    result = (((*data) << 24)&0x000000FF) | (((*data) << 8)&0x0000FF00) | (((*data) >> 8)&0x00FF0000) | (((*data) >> 24)&0xFF000000);
    return result;
}

void Protocol_ParsePacket(); // deals with ping and pong. and removes packets from buffer.
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
            if (byte == rxPacket -> checkSum) {
                state = STATE_END_R;
                break;
            } else {
                state = STATE_HEAD; // fill in errors here.
                break;
            }
            break;
        case STATE_END_R:
            if (byte == '\r') {
                state = STATE_END_N;
                break;
            } else {
                state = STATE_HEAD; // fill in errors here.
                break;
            }
            break;
        case STATE_END_N:
            if (byte == '\n') {
                state = STATE_HEAD;
                break;
            } else {
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
unsigned char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned char curChecksum) {
    curChecksum = (curChecksum >> 1) + (curChecksum << 7);
    curChecksum += charIn;
    return curChecksum;
}

// CBuffer_init()
// initializes a new circle buffer.

rxpBuffObj RXBuffer_init() {
    rxpBuffObj RX = malloc(sizeof (rxpBuffObj));
    assert(RX != NULL);
    RX -> head = 0;
    RX -> tail = 0;
    RX -> buffer = calloc(PACKETBUFFERSIZE, sizeof (rxpADT));
    return RX;
}

// CBuffer_init()
// initializes a new circle buffer.

rxpADT newPacket() {
    rxpADT rxPacket = malloc(sizeof (rxpADT));
    assert(rxPacket != NULL);
    rxPacket -> checkSum = 0;
    rxPacket -> len = 0;
    rxPacket -> payLoad = calloc(MAXPAYLOADLENGTH, sizeof (unsigned char));
    return rxPacket;
}

// freeCBuffer()
// frees all heap memory associated with circle buffer. Set pCB to Null.

void freeRXPacket(rxpADT *pRX) {
    if (pRX != NULL && (*pRX) != NULL) {
        free(((*pRX) -> payLoad));
        (*pRX) -> payLoad = NULL;
        free(*pRX);
        (*pRX) = NULL;
    }
}

// freeCBuffer()
// frees all heap memory associated with circle buffer. Set pCB to Null.

void freeRXBuffer(rxpBuffObj *pRX) {
    if (pRX != NULL && (*pRX) != NULL) {
        free(((*pRX) -> buffer));
        (*pRX) -> buffer = NULL;
        free(*pRX);
        (*pRX) = NULL;
    }
}

// CB_isFull()
// checks if the buffer is full.

int RX_isFull(rxpBuffObj RX) {
    return (RX -> tail + 1) % PACKETBUFFERSIZE == RX -> head;
}

// CB_isEmpty())
// checks if the buffer is empty.

int RX_isEmpty(rxpBuffObj RX) {
    return RX -> head == RX -> tail;
}

// WritetoCB()
// writes to the circular buffer.

int WritetoRX(rxpBuffObj RX, rxpADT data) {
    if (RX_isFull(RX)) {
        return true;
    } else {
        RX -> buffer[RX -> tail] = data;
        RX -> tail = (RX -> tail + 1) % PACKETBUFFERSIZE;
    }
}

// ReadtoCB()
// writes to the circular buffer.

rxpADT ReadfromRX(rxpBuffObj RX) {
    if (RX_isEmpty(RX)) {
        return 0;
    } else {
        rxpADT data = RX -> buffer[RX -> head];
        RX -> head = (RX -> head + 1) % PACKETBUFFERSIZE;
        return data;
    }
}

#ifdef Part3
void main(){
    BOARD_Init();
    Protocol_Init(115200);
    LEDS_INIT();
    
    rxpADT TestPacket = newPacket();
    unsigned char check = 0;
    check = Protocol_CalcIterativeChecksum(0x81, check);
    check = Protocol_CalcIterativeChecksum(0xFF, check);
    assert(check == 0xBF);
    asm("NOP");
    
    unsigned char buildpacket[] = {204, 2, 129, 0, 185, 192, 13, 10};
    for (int i = 0; i < 7; i++){
        BuildRxPacket(TestPacket, buildpacket[i]);
    }
    freeRXPacket(&TestPacket); 
}
#endif
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
#include <string.h>
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

typedef enum state_errors {
    ERROR_NONE,
    NULL_PACKET,
    STATE_HEAD_NOT_HEAD,
    STATE_LEN_NOT_LEN,
    STATE_ID_NOT_ID,
    STATE_PAYL_NOT_PAYL,
    STATE_TAIL_NOT_TAIL,
    STATE_CHECKSUM_NOT_CHECK,
    STATE_END_R_NOT_R,
    STATE_END_N_NOT_N
} state_errors;

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

static state_errors state_e;
static PacketState state;
static rxpBuffObj RX;
static rxpADT rxPacket;
static int count;
static int flag;
static int RX_Buff_full;
static int rx_count;
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
void Protocol_ParsePacket();
int GetID(rxpADT packet);
int GetLen(rxpADT packet);
/*******************************************************************************
 ******************************************************************************/

/**
 * @Function Protocol_Init(baudrate)
 * @param Legal Uart baudrate
 * @return SUCCESS (true) or ERROR (false)
 * @brief Initializes Uart1 for stream I/O to the lab PC via a USB 
 *        virtual comm port. Baudrate must be a legal value. 
 * @author instructor W2022 */
int Protocol_Init(unsigned long baudrate) {
    Uart_Init(baudrate);
    RX = RXBuffer_init();
    rxPacket = NULL;
    if (RX == NULL) {
        return 0;
    }
    state = STATE_HEAD;
    state = ERROR_NONE;
    rx_count = 0;
    flag = 0;
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
    if (u1rx_isEmpty()) {
        return false;
    }
    if (RX_isFull(RX)) {
        return 1;
    }
    if (rxPacket == NULL) {
        rxPacket = newPacket();
    }
    unsigned char c = GetChar();
    BuildRxPacket(rxPacket, c);
    char msg[MAXPAYLOADLENGTH];
    sprintf(msg, "%d\n", state_e);
    Protocol_SendDebugMessage(msg);
    if (flag == 1) {
        flag = 0;
        WritetoRX(RX, rxPacket);
        rxPacket = NULL;
    }
}

/**
 * @Function int Protocol_GetInPacket(uint8_t *type, uint8_t *len, uchar *msg)
 * @param *type, *len, *msg
 * @return SUCCESS (true) or WAITING (false)
 * @brief Reads the next packet from the packet Buffer 
 * @author instructor W2022 */
int Protocol_GetInPacket(uint8_t *type, uint8_t *len, unsigned char *msg) {
    if (RX_isEmpty(RX)) {
        return 0; // make an error for this to return
    }
    rxpADT getpacket = ReadfromRX(RX);
    *type = GetID(getpacket);
    *len = GetLen(getpacket);
    for (int i = 1; i < GetLen(getpacket); i++) {
        msg[i-1] = getpacket -> payLoad[i-1];

    }
    freeRXPacket(&getpacket);
    return 1;
}

/**
 * @Function int Protocol_SendDebugMessage(char *Message)
 * @param Message, Proper C string to send out
 * @return SUCCESS (true) or ERROR (false)
 * @brief Takes in a proper C-formatted string and sends it out using ID_DEBUG
 * @warning this takes an array, do <b>NOT</b> call sprintf as an argument.
 * @author mdunne */
int Protocol_SendDebugMessage(char *Message) {
    unsigned char length = strlen(Message);
    return Protocol_SendPacket(length, ID_DEBUG, (unsigned char *) Message);
}

/**
 * @Function int Protocol_SendPacket(unsigned char len, void *Payload)
 * @param len, length of full <b>Payload</b> variable
 * @param Payload, pointer to data
 * @return SUCCESS (true) or ERROR (false)
 * @brief composes and sends a full packet
 * @author instructor W2022 */
int Protocol_SendPacket(unsigned char len, unsigned char ID, unsigned char *Payload) {
    uint8_t checks = 0;
    PutChar(HEAD);
    PutChar(len);
    PutChar(ID);
    checks = Protocol_CalcIterativeChecksum(ID, checks);
    for (int i = 0; i < (len-1); i++) {
        PutChar(Payload[i]);
        checks = Protocol_CalcIterativeChecksum(Payload[i], checks);
    }
    PutChar(TAIL);
    PutChar(checks);
    PutChar('\r');
    PutChar('\n');
    return 1;
}

/**
 @Function unsigned char Protocol_ReadNextID(void)
 * @param None
 * @return Reads the ID of the next available Packet
 * @brief Returns ID or 0 if no packets are available
 * @author instructor W2022 */
unsigned char Protocol_ReadNextPacketID(void) {
    rxpADT packet = RX -> buffer[RX -> head];
    if (packet == NULL) {
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
void flushPacketBuffer() {
    for (int i = 0; i <= PACKETBUFFERSIZE; i++) {
        freeRXPacket(&(RX -> buffer[i]));
    }
    RX -> head = 0;
    RX -> tail = 0;
}

unsigned int convertEndian(unsigned int* data) {
    unsigned int result;
//    result = (((*data)&0x000000FF) >> 24) | (((*data)&0x0000FF00) << 8) | (((*data)&0x00FF0000) >> 8) | (((*data)&0xFF000000) << 24);
    result = (((*data)&0x000000FF) << 24) | (((*data)&0x0000FF00) << 8) | (((*data)&0x00FF0000) >> 8) | (((*data)&0xFF000000) >> 24);
    return result;
}

void Protocol_ParsePacket() { // deals with ping and pong. and removes packets from buffer.
    uint8_t type_for_parsepacket;
    uint8_t length_for_parsepacket;
    unsigned char msg[MAXPAYLOADLENGTH];
    if (RX_isEmpty(RX)) {
        return;
    }
    Protocol_GetInPacket(&type_for_parsepacket, &length_for_parsepacket, msg);
    if (type_for_parsepacket == ID_PING) {
//        int number = *((int*)msg);
//        number = convertEndian(&number);
//        NOP();
//        number >>= 1;
//        NOP();
//        number = convertEndian(&number);
//        Protocol_SendPacket(5, ID_PONG, &number);
        unsigned char c0 = msg[0];
        unsigned char c1 = msg[1];
        unsigned char c2 = msg[2];
        unsigned char c3 = msg[3];
        
        unsigned int num1 = c0 << 24;
        unsigned int num2 = c1 << 16;
        unsigned int num3 = c2 << 8;
        unsigned int num4 = c3;
        
        unsigned int val = num1 | num2 | num3 | num4;
        
        val = val >> 1;
        
        c0 = (val&0xFF000000) >> 24;
        c1 = (val&0x00FF0000) >> 16;
        c2 = (val&0x0000FF00) >> 8;
        c3 = (val&0x000000FF);
        
        unsigned char message[MAXPAYLOADLENGTH];
        message[0] = c0;
        message[1] = c1;
        message[2] = c2;
        message[3] = c3;
        Protocol_SendPacket(5, ID_PONG, message);
//        unsigned int val = (unsigned int)msg[1]
//        int* val = malloc(sizeof (int));
//        unsigned char* bit_num = val;
//        for (int i = 0; (i < ((length_for_parsepacket) - 1)); i++) {
//            *bit_num = msg[i];
//            bit_num++;
//        }
//        
//        *val = convertEndian(val);
//        *val = *val >> 1;
//        *val = convertEndian(val);
//        unsigned char byte0 = (*val & 0x000000FF);
//        unsigned char byte1 = (*val & 0x0000FF00) >> 8;
//        unsigned char byte2 = (*val & 0x00FF0000) >> 16;
//        unsigned char byte3 = (*val & 0xFF000000) >> 24;
//        unsigned char message[] = {byte0, byte1, byte2, byte3};
//        Protocol_SendPacket(5, ID_PONG, &message[0]);
//        free(val);
    }
    if (type_for_parsepacket == ID_LEDS_GET) {
        unsigned char payl = LEDS_GET();
        Protocol_SendPacket(2, ID_LEDS_STATE, &payl);
    }
    if (type_for_parsepacket == ID_LEDS_SET) {
        LEDS_SET(msg[0]);
    }
}
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
 *   rxpADT rxPacket ..a.
 * Now consider how to create another structure for use as a circular buffer
 * containing a PACKETBUFFERSIZE number of these rxpT packet structures.
 ******************************************************************************/
uint8_t BuildRxPacket(rxpADT rxPacket, unsigned char byte) {
    if (rxPacket == NULL){
        return state_e = NULL_PACKET;
    }
    switch (state) {
        case STATE_HEAD:
            count = 0;
            if (byte == HEAD) {
                rxPacket-> checkSum = 0;
                for (int i = 0; i <= MAXPAYLOADLENGTH; i++) {
                    rxPacket -> payLoad[i] = 0;
                }
                state = STATE_LEN;
            } else {
                state = STATE_HEAD; // fill in errors here.
                state_e = STATE_HEAD_NOT_HEAD;
            }
            break;
        case STATE_LEN:
            if (byte <= MAXPAYLOADLENGTH) {
                rxPacket -> len = byte;
                state = STATE_ID;
            } else {
                state = STATE_HEAD;
                state_e = STATE_LEN_NOT_LEN;
            }
            break;
        case STATE_ID:
            if (byte) {
                rxPacket -> ID = byte;
                rxPacket -> checkSum = Protocol_CalcIterativeChecksum(byte, rxPacket -> checkSum);
                state = STATE_PAYL;
                count++;
                if (rxPacket -> len == 1) {
                    state = STATE_TAIL;
                }
            } else {
                state = STATE_HEAD; // fill in errors here.
                state_e = STATE_ID_NOT_ID;
            }
            break;
        case STATE_PAYL:;
            rxPacket -> checkSum = Protocol_CalcIterativeChecksum(byte, rxPacket -> checkSum);
            rxPacket -> payLoad[count - 1] = byte;
            count++;
            if (count == (GetLen(rxPacket))) {
                state = STATE_TAIL;
            }
            break;
        case STATE_TAIL:
            if (byte == TAIL) {
                state = STATE_CHECKSUM;
            } else {
                state = STATE_HEAD; // fill in errors here.
                state_e = STATE_TAIL_NOT_TAIL;
            }
            break;
        case STATE_CHECKSUM:
            if (byte == rxPacket -> checkSum) {
                state = STATE_END_R;
            } else {
                state = STATE_HEAD; // fill in errors here.
                state_e = STATE_CHECKSUM_NOT_CHECK;
            }
            break;
        case STATE_END_R:
            if (byte == '\r') {
                state = STATE_END_N;
            } else {
                state = STATE_HEAD; // fill in errors here.
                state_e = STATE_END_R_NOT_R;
            }
            break;
        case STATE_END_N:
            if (byte == '\n') {
                state = STATE_HEAD;
                flag = 1;
            } else {
                state = STATE_HEAD; // fill in errors here.
                state_e = STATE_END_N_NOT_N;
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

int GetID(rxpADT packet) {
    return packet -> ID;
}

int GetLen(rxpADT packet) {
    return packet -> len;
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
    return((RX -> tail + 1) % PACKETBUFFERSIZE == RX -> head);
}

// CB_isEmpty())
// checks if the buffer is empty.

int RX_isEmpty(rxpBuffObj RX) {
    return (RX -> head == RX -> tail);
}

// WritetoCB()
// writes to the circular buffer.

int WritetoRX(rxpBuffObj RX, rxpADT data) {
    if (!RX_isFull(RX)) {
        RX -> buffer[RX -> tail] = data;
        RX -> tail = (RX -> tail + 1) % PACKETBUFFERSIZE;
    }
}

// ReadtoCB()
// writes to the circular buffer.

rxpADT ReadfromRX(rxpBuffObj RX) {
    if (!RX_isEmpty(RX)) {
        rxpADT data = RX -> buffer[RX -> head];
        RX -> head = (RX -> head + 1) % PACKETBUFFERSIZE;
        return data;
    }
}

#ifdef Part3

void main() {
    BOARD_Init();
    Protocol_Init(115200);
    LEDS_INIT();

    rxpADT TestPacket = newPacket();
    unsigned char check = 0;
    check = Protocol_CalcIterativeChecksum(0x81, check);
    check = Protocol_CalcIterativeChecksum(0xFF, check);
    assert(check == 0xBF);
    //asm("NOP");

    //Set led
    unsigned char buildpacket[] = {204, 2, 129, 255, 185, 191, 13, 10};

    //Get led
    unsigned char buildpacket1[] = {204, 1, 131, 185, 131, 13, 10};
    
    //Failed Ping
//    unsigned char buildpacket2[] = {204, 5, 132, 221, 147, 68, 251, 185, 165, 13, 10};
    unsigned char buildpacket3[] = {204, 5, 132, 244, 124, 50, 163, 185, 161, 13, 10};
//    for (int i = 0; i <= 6; i++) {
//        BuildRxPacket(TestPacket, buildpacket1[i]);
//    }
//    WritetoRX(RX, TestPacket);
//    Protocol_ParsePacket();
//    // state machine test
//    for (int i = 0; i <= 7; i++) {
//        BuildRxPacket(TestPacket, buildpacket[i]);
//    }
//    WritetoRX(RX, TestPacket);
//    Protocol_ParsePacket();
//    for (int i = 0; i <= 10; i++) {
//        BuildRxPacket(TestPacket, buildpacket2[i]);
//    }
//    WritetoRX(RX, TestPacket);
//    Protocol_ParsePacket();
//    for (int i = 0; i <= 10; i++) {
//        BuildRxPacket(TestPacket, buildpacket3[i]);
//    }
//    WritetoRX(RX, TestPacket);
//    Protocol_ParsePacket();
    //    unsigned char length = 0x02;
    //    unsigned char ids = 0x81;
    //    unsigned char lights = 0xFF;
    //    Protocol_SendPacket(length, ids, &lights);
    //    
    //    unsigned char length1 = 0x02;
    //    unsigned char ids1 = 0x82;
    //    unsigned char lights1 = LEDS_GET();
    //    Protocol_SendPacket(length1, ids1, &lights1);
    while (1) {
        Protocol_QueuePacket();
        Protocol_ParsePacket();
    }
}
#endif
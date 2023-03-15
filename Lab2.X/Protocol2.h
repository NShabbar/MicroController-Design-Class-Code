/* 
 * File:   Protocol2.h
 * Author: Nadia Shabbar
 * rewritten for Uart.c ECE121 W2023
 */
#ifndef PROTOCOL2_H
#define PROTOCOL2_H
/*******************************************************************************
 * PUBLIC #DEFINES                                                            *
 ******************************************************************************/
#define PACKETBUFFERSIZE 16
#define MAXPAYLOADLENGTH 128 
#define DEBUG 0 

#define HEAD 0xCC
#define TAIL 0xB9

/*******************************************************************************
 * PUBLIC DATATYPES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/
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
int Protocol_SendPacket(unsigned char len, unsigned char ID, unsigned char *Payload);
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
unsigned int convertEndian(unsigned int* data);

void Protocol_ParsePacket();

#define LEDS_INIT() do {LATECLR = 0xFF; TRISECLR = 0xFF;} while (0)
/**
 * Provides a way to quickly get the status of all 8 LEDs into a uint8, where a bit
is 1 if the LED
 * is on and 0 if it's not. The LEDs are ordered such that bit 7 is LED8 and bit 0 
is LED0.
 */
#define LEDS_GET() (LATE & 0xFF)
/**
 * This macro sets the LEDs on according to which bits are high in the argument. 
Bit 0 corresponds
 * to LED0.
 * @param leds Set the LEDs to this value where 1 means on and 0 means off.
 */
#define LEDS_SET(leds) do { LATE = (leds); } while (0)
#endif /* PROTOCOL_H */


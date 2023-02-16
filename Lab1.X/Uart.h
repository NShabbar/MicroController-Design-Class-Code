/***
 * File:    uart.h
 * Author:  Nadia
 * Created: ECE121 W2023 rev 1
 * This library implements a true UART device driver that enforces 
 * I/O stream abstraction between the physical and application layers.
 * All stream accesses are on a per-character or byte basis. 
 */

#ifndef UART_H
#define UART_H
/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/
/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/
//unsigned char u1rx_isEmpty(void);
/**
 * @Function Uart_Init(unsigned long baudrate)
 * @param baudrate
 * @return none
 * @brief  Initializes UART1 to baudrate N81 and creates circ buffers
 * @author instructor ece121 W2022 */
int Uart_Init(unsigned long baudRate);

/**
 * @Function int PutChar(char ch)
 * @param ch - the character to be sent out the serial port
 * @return True if successful, else False if the buffer is full or busy.
 * @brief  adds char to the end of the TX circular buffer
 * @author instrutor ECE121 W2022 */
int PutChar(unsigned char *ch);

/**
 * @Function unsigned char GetChar(void)
 * @param None.
 * @return NULL for error or a Char in the argument.
 * @brief  dequeues a character from the RX buffer,
 * @author instructor, ECE121 W2022 */
int GetChar(unsigned char* data);
//int GetChar(unsigned char* data);

#endif // UART_H

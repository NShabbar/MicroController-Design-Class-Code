/***
 * File:    CircularBuffer.h
 * Author:  Nadia
 * Created: ECE121 W2023 rev 1
 * This library implements a circular buffer for the Uart to receive chars. 
 */

#ifndef _CircularBuffer_H    /* Guard against multiple inclusion */
#define _CircularBuffer_H  
#include<stdio.h>
#include<stdbool.h>
#include "BOARD.h"

// Exported types -------------------------------------------------------------
//typedef int CBElement;
typedef struct CircBuffObj* CBuffer;

//CBuffer_init()
// initializes a new circle buffer.
CBuffer CBuffer_init();

//freeCBuffer())
// frees all heap memory associated with circle buffer. Set pCB to Null.
void freeCBuffer(CBuffer* pCB);

// CB_isFull()
// checks if the buffer is full.
int CB_isFull(CBuffer CB);

// CB_isEmpty())
// checks if the buffer is empty.
int CB_isEmpty(CBuffer CB);

// WritetoCB()
// writes to the circular buffer.
int WritetoCB(CBuffer CB, unsigned char data);

// ReadtoCB()
// writes to the circular buffer.
unsigned char ReadfromCB(CBuffer CB);
#endif
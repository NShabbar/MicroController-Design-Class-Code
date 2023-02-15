/***
 * File:    CircularBuffer.c
 * Author:  Nadia
 * Created: ECE121 W2023 rev 1
 * This library implements a circular buffer for the Uart to receive chars. 
 */

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<stdbool.h>
#include "CircularBuffer.h"
#include "BOARD.h"
#include <xc.h>
#include <math.h>
#include <sys/attribs.h>

//#define Test
#define BUFFER_SIZE 16
#define NOPS_FOR_5_MS 3125 //measured by the oscilloscope to make 5 ms.

void NOP() {
    int i;
    for (i = 0; i < NOPS_FOR_5_MS; i++) {
        asm("nop");
    }
}

// structs --------------------------------------------------------------------

// private GraphObj type

typedef struct CircBuffObj {
    int head;
    int tail;
    char *buffer;
} CircBuffObj;

// Constructors-Destructors ---------------------------------------------------

// CBuffer_init()
// initializes a new circle buffer.

CBuffer CBuffer_init() {
    CBuffer CB = malloc(sizeof (CircBuffObj));
    assert(CB != NULL);
    CB -> head = 0;
    CB -> tail = 0;
    CB -> buffer = calloc(BUFFER_SIZE, sizeof (char));
    return CB;
}

// freeCBuffer()
// frees all heap memory associated with circle buffer. Set pCB to Null.

void freeCBuffer(CBuffer *pCB) {
    if (pCB != NULL && (*pCB) != NULL) {
        free(((*pCB) -> buffer));
        (*pCB) -> buffer = NULL;
        free(*pCB);
        (*pCB) = NULL;
    }
}

// CB_isFull()
// checks if the buffer is full.

int CB_isFull(CBuffer CB) {
    return (CB -> tail + 1) % BUFFER_SIZE == CB -> head;
}

// CB_isEmpty())
// checks if the buffer is empty.

int CB_isEmpty(CBuffer CB) {
    return CB -> head == CB -> tail;
}

// WritetoCB()
// writes to the circular buffer.

int WritetoCB(CBuffer CB, unsigned char data) {
    if (CB_isFull(CB)) {
        return true;
    } else {
        CB -> buffer[CB -> tail] = data;
        CB -> tail = (CB -> tail + 1) % BUFFER_SIZE;
    }
}

// ReadtoCB()
// writes to the circular buffer.

unsigned char ReadfromCB(CBuffer CB) {
    if (CB_isEmpty(CB)) {
        return 0;
    } else {
        char data = CB -> buffer[CB -> head];
        CB -> head = (CB -> head + 1) % BUFFER_SIZE;
        return data;
    }
}

// Testing circular buffer
#ifdef Test
void main(){
    BOARD_Init();
    CBuffer CB = CBuffer_init();
    bool empty = CB_isEmpty(CB);
    assert(empty == true);
    for (int i = 0; i <= BUFFER_SIZE + 1; i++){
        WritetoCB(CB, 's');
    }
    bool full = CB_isFull(CB);
    assert(full == true);
    while (!CB_isEmpty(CB)){
        char read = ReadfromCB(CB);
        assert(read == 's');
    }
    NOP();
}
#endif

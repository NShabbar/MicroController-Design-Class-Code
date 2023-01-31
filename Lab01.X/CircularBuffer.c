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

#define BUFFER_SIZE 512


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
    return (CB -> head + 1) % BUFFER_SIZE == CB -> tail;
}

// CB_isEmpty())
// checks if the buffer is empty.

int CB_isEmpty(CBuffer CB) {
    return CB -> head == CB -> tail;
}

// WritetoCB()
// writes to the circular buffer.

void WritetoCB(CBuffer CB, char data) {
    if (CB_isFull(CB)) {
        printf("\nBuffer is Full.");
        return;
    } else {
        CB -> buffer[CB -> tail] = data;
        CB -> tail = (CB -> tail + 1) % BUFFER_SIZE;
    }
}

// ReadtoCB()
// writes to the circular buffer.

void ReadtoCB(CBuffer CB) {
    if (CB_isEmpty(CB)) {
        printf("\nBuffer is Empty.");
        return -1;
    } else {
        char data = CB -> buffer[CB -> head];
        CB -> head = (CB -> head + 1) % BUFFER_SIZE;
        return data;
    }
}

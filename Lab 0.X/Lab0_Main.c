/* 
 * File:   Main.c
 * Author: Nadia
 *
 * Created on January 11, 2023, 1:15 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include "BOARD.h"
#include "LEDs.h"

int newPattern = 0x00;
#define LEDS LATE
#define NOPS_FOR_5_MS 5000

void NOP() {
    int i;
    for (i = 0; i < NOPS_FOR_5_MS; i++) {
        asm("nop");
    }
}

void main() {

    LEDS_INIT();
    while (1) {
        LEDS = newPattern;
        newPattern++;
        if (BTN1 || BTN2 || BTN3 || BTN4) {
            newPattern = 0x00;
        }
        if (newPattern >= 0xFF) {
            newPattern = 0x00;
        }
        NOP();
    }
}




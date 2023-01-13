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

#define LEDS LATE
#define NOPS_FOR_5_MS 3125 //measured by the oscilloscope to make 5 ms.

void NOP() {
    int i;
    for (i = 0; i < NOPS_FOR_5_MS; i++) {
        asm("nop");
    }
}

void ButtonsInit(void) {
    TRISD |= 0x00E0; //set to input
    TRISF |= 0x0020; //also set to input
}

void main() {
    BOARD_Init();
    LEDS_INIT();
    int LED_counter = 0x00;
    int NOP_counter = 0;
    while (1) {
        LEDS = LED_counter;
        if (NOP_counter == 100) { //When counter == 50, it is 250ms but 32s long. At counter == 100, it is 1 min. and 4 secs, but 500ms.
            LED_counter++;
            NOP_counter = 0;
        }
        if (BTN1 || BTN2 || BTN3 || BTN4) {
            LED_counter = 0x00;
        }
        if (LED_counter >= 0xFF) {
            LED_counter = 0x00;
        }
        NOP();
        NOP_counter++;
    }
}

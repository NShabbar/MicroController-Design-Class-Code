/* 
 * File:   Lab0_parta.c
 * Author: Nadia
 *
 * Created on January 13, 2023, 11:23 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include "BOARD.h"
#include "LEDs.h"

#define LEDS LATE

void ButtonsInit(void) {
    TRISD |= 0x00E0; //set to input
    TRISF |= 0x0020; //also set to input
}

void main() {
    BOARD_Init();
    LEDS_INIT();
    while (1) {
        if (BTN1) {
            LEDS = 0b00000011;
        }
        else if (BTN2) {
            LEDS = 0b00001100;
        }
        else if (BTN3) {
            LEDS = 0b00110000;
        }
        else if (BTN4) {
            LEDS = 0b11000000;
        }
        else{
            LEDS = 0x00;
        }
    }
}


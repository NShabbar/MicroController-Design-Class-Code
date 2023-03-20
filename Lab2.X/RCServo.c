#include "BOARD.h"
#include <xc.h>
#include <math.h>
#include<assert.h>
#include <sys/attribs.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "Protocol2.h"
#include "RCServo.h"

static int PULSE;
/**
 * @Function RCServo_Init(void)
 * @param None
 * @return SUCCESS or ERROR
 * @brief initializes hardware required and set it to the CENTER PULSE */
int RCServo_Init(void){
    T3CON = 0;
    TMR3 = 0;
    PR3 = 0;
    OC3CON = 0;
    OC3R = 0;
    OC3RS = 0;
    T3CONbits.TCKPS = 5; //1:32
    PR3 = 62499;
    OC3CONbits.OCTSEL = 1; //Timer3 is the clock source for this Output Compare module
    OC3CONbits.OCM = 5; //Initialize OCx pin low; generate continuous output pulses on OCx pin
    OC3R = 12000;
    PULSE = RC_SERVO_CENTER_PULSE;
    OC3RS = OC3R + (PULSE * 1.25);
    IEC0bits.OC3IE = 1; //int enable
    IPC3bits.OC3IP = 4; //prio
    IPC3bits.OC3IS = 2; //subprio
    T3CONbits.ON = 1; //turn on
    OC3CONbits.ON = 1; //turn on
    return SUCCESS;
}

/**
 * @Function int RCServo_SetPulse(unsigned int inPulse)
 * @param inPulse, integer representing number of microseconds
 * @return SUCCESS or ERROR
 * @brief takes in microsecond count, converts to ticks and updates the internal variables
 * @warning This will update the timing for the next pulse, not the current one */
int RCServo_SetPulse(unsigned int inPulse);

/**
 * @Function int RCServo_GetPulse(void)
 * @param None
 * @return Pulse in microseconds currently set */
unsigned int RCServo_GetPulse(void);

/**
 * @Function int RCServo_GetRawTicks(void)
 * @param None
 * @return raw timer ticks required to generate current pulse. */
unsigned int RCServo_GetRawTicks(void);


void __ISR(_OUTPUT_COMPARE_3_VECTOR) __OC3Interrupt(void);
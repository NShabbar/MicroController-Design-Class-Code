#include "BOARD.h"
#include <xc.h>
#include <math.h>
#include<assert.h>
#include <sys/attribs.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "Protocol2.h"

/**
 * @Function DCMotorDrive_Init(void)
 * @param None
 * @return SUCCESS or ERROR
 * @brief initializes timer3 to 2Khz and set up the pins
 * @warning you will need 3 pins to correctly drive the motor  */
int DCMotorDrive_Init(void){
    // the 32-bit as the clock source for the compare time base.
    T3CON = 0x0018; // Configure Timer2 for 32-bit operation
    // with a prescaler of 2. The Timer2/Timer3
    // pair is accessed via registers associated
    // with the Timer2 register
    OC3CON = 0x0000; // Turn off OC1 while doing setup.
    OC3CON = 0x0023; // Configure for compare toggle mode
    OC3R = 0x00500000; // Initialize Compare Register 1
    PR3 = 0x00500000; // Set period (PR2 is now 32-bits wide)
    // configure int
    IFS0CLR = 0x00000040; // Clear the OC1 interrupt flag
    IFS0SET = 0x00000040; // Enable OC1 interrupt
    IPC3SET = 0x001C0000; // Set OC1 interrupt priority to 7,
    // the highest level
    IPC3SET = 0x00030000; // Set Subpriority to 3, maximum
    T3CONSET = 0x8000; // Enable Timer2
    OC3CONSET = 0x8000; // Enable OC1
}


/**
 * @Function DCMotorDrive_SetMotorSpeed(int newMotorSpeed)
 * @param newMotorSpeed, in units of Duty Cycle (+/- 1000)
 * @return SUCCESS or ERROR
 * @brief Sets the new duty cycle for the motor, 0%->0, 100%->1000 */
int DCMotorDrive_SetMotorSpeed(int newMotorSpeed);


/**
 * @Function DCMotorControl_GetMotorSpeed(void)
 * @param None
 * @return duty cycle of motor 
 * @brief returns speed in units of Duty Cycle (+/- 1000) */
int DCMotorControl_GetMotorSpeed(void);


/**
 * @Function DCMotorDrive_SetBrake(void)
 * @param None
 * @return SUCCESS or FAILURE
 * @brief set the brake on the motor for faster stop */
int DCMotorDrive_SetBrake(void);


void __ISR(_OUTPUT_COMPARE_3_VECTOR) __OC3Interrupt(void){
    // Insert user code here
    IFS0CLR = 0x0040; // Clear the OC1 interrupt flag
}
#include "BOARD.h"
#include <xc.h>
#include <math.h>
#include<assert.h>
#include <sys/attribs.h>
#include <string.h>
#include "Protocol2.h"

static unsigned int milliseconds;
static unsigned int microseconds;

/**
 * @Function FreeRunningTimer_Init(void)
 * @param none
 * @return None.
 * @brief  Initializes the timer module */
void FreeRunningTimer_Init(void){
    microseconds = 0;
    milliseconds = 0;
    BOARD_Init();
    T5CON = 0x0; // Stop any 16-bit Timer5 operation 
    PR5 = 9999; // period register
    TMR5 = 0; // counter register
    T5CONbits.TCKPS = 2; // The TCKPS<2:0> bits are available only on even numbered Type B timers. For example, Timer2 and Timer4 in 32-bit Timer mode
    T5CONbits.ON = 1; // enable
    IPC5bits.T5IP = 3; // priority
    IPC5bits.T5IS = 0; // sub-priority
    IFS0bits.T5IF = 0; // interrupt flag
    IEC0bits.T5IE = 1; // interrupt enable
    // internal peripheral clock source
}

/**
 * Function: FreeRunningTimer_GetMilliSeconds
 * @param None
 * @return the current MilliSecond Count
   */
unsigned int FreeRunningTimer_GetMilliSeconds(void){
    return milliseconds;
}

/**
 * Function: FreeRunningTimer_GetMicroSeconds
 * @param None
 * @return the current MicroSecond Count
   */
unsigned int FreeRunningTimer_GetMicroSeconds(void){
    microseconds = 1000 * milliseconds;
    return microseconds;
}

void __ISR(_TIMER_5_VECTOR, ipl3auto) Timer5IntHandler(void){
    milliseconds++;
    IFS0bits.T5IF = 0;
}

#ifdef Lab2Part1
int main(){
    FreeRunningTimer_Init();
    Protocol_Init(111520);
    TRISE = 0;
    LATE = 0;
    while(1){
        if ((milliseconds % 2000) == 0){
            LATE ^= 0x01;
        }
    }
}
#endif
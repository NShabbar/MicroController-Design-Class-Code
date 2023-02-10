/***
 * File:    uart.c
 * Author:  Nadia
 * Created: ECE121 W2023 rev 1
 * This library implements a true UART device driver that enforces 
 * I/O stream abstraction between the physical and application layers.
 * All stream accesses are on a per-character or byte basis. 
 */
#include "Uart.h"
#include "BOARD.h"
#include <xc.h>
#include <math.h>
#include <sys/attribs.h>
#include "CircularBuffer.h"

//#define Part1
#define Part2

static CBuffer U1RX_buffer;
static CBuffer U1TX_buffer;


int Uart_Init(unsigned long baudRate) {
    U1RX_buffer = CBuffer_init();
    U1TX_buffer = CBuffer_init();
    if(U1RX_buffer == NULL || U1TX_buffer == NULL){
        return 0;
    }
    // Clear all Registers
    U1MODE = 0; // initialize Uart1 mode register to 0.
    U1STA = 0; // initialize Uart1 status and control register to 0.
    U1TXREG = 0; // intialize Uart1 transmit register to 0.
    U1RXREG = 0; // intialize Uart1 receive register to 0.

    //Calculate Baude Rate
    unsigned int Fpb = BOARD_GetPBClock(); // May need to round with math.h round functions.
    U1BRG = (Fpb / (16 * baudRate)) - 1; // PBCLK = 40 MHz.
    // To calculate U1BRG given Baud Rate = 115200, use function:
    // UxBRG = (Fpb/(16*Baud Rate)) - 1, where Fpb is the frequency of the PBCLK.
    // PBCLK is the board's clock from the reference manual.

    // Enable U1MODE bits with 8 bit no-parity, the most common parity.
    U1MODEbits.PDSEL = 00;

    // Enable UART1
    U1MODEbits.ON = 1; // ON, turns the UART on.

    // Enable
    U1STAbits.URXEN = 1; // Enables the receiver bit.
    U1STAbits.UTXEN = 1; // Enables the transmission bit.
    U1STAbits.UTXISEL = 1; //interrupt when transmission is complete
    U1STAbits.URXISEL = 0; //interrupt when RX is not empty (has at least 1 character)
    // UxSTA is used because it is the status and control register.
    // It controls the status of the other two registers.

    // Enable Interrupts
    IEC0bits.U1RXIE = 1; // Enable interrupt
    IEC0bits.U1TXIE = 1; // Enable interrupt
    IFS0bits.U1TXIF = 0;
    IFS0bits.U1RXIF = 0;

    // Interrupt Priorities
    IPC6bits.U1IP = 4; // Prio
    IPC6bits.U1IS = 3; // subprio
    return 1;
}

/**
 * Refer to ...\docs\MPLAB C32 Libraries.pdf: 32-Bit Language Tools Library.
 * In sec. 2.13.2 helper function _mon_putc() is noted as normally using
 * UART2 for STDOUT character data flow. Adding a custom version of your own
 * can redirect this to UART1 by calling your putchar() function.   
 */
void _mon_putc(char c) {
    PutChar(c);
}

/****************************************************************************
 * Function: IntUart1Handler
 * Parameters: None.
 * Returns: None.
 * The PIC32 architecture calls a single interrupt vector for both the 
 * TX and RX state machines. Each IRQ is persistent and can only be cleared
 * after "removing the condition that caused it". This function is declared in
 * sys/attribs.h. 
 ****************************************************************************/
void __ISR(_UART1_VECTOR) IntUart1Handler(void) {
    if (IFS0bits.U1RXIF) {
        IFS0bits.U1RXIF = 0;
        while(U1STAbits.URXDA){
           unsigned char read_data = U1RXREG;
           WritetoCB(U1RX_buffer, read_data); 
        }  
    }
    if (IFS0bits.U1TXIF) {
        IFS0bits.U1TXIF = 0;
        while(!U1STAbits.UTXBF && !CB_isEmpty(U1TX_buffer)){
            unsigned char write_data = ReadfromCB(U1TX_buffer);
            U1TXREG = write_data;
        }
    }
}

int PutChar(char ch) {
    if (CB_isFull(U1TX_buffer) == true) {
        return false;
    }
    if (!CB_isFull(U1TX_buffer)) {
        IEC0bits.U1TXIE = 0;
        WritetoCB(U1TX_buffer, ch);
        IEC0bits.U1TXIE = 1;
        //IFS0bits.U1TXIF = 1;
    }
    if (U1STAbits.TRMT && !CB_isEmpty(U1TX_buffer)) {
        IFS0bits.U1TXIF = 1;
    }
}

unsigned char GetChar(void) {
    unsigned char data = ReadfromCB(U1RX_buffer);
    return data;
}

#ifdef Part1

void main() {
    BOARD_Init();
    Uart_Init(115200);
    while (1) {
        if (U1STAbits.URXDA == 1) {
            U1TXREG = U1RXREG;
        }
    }
}
#endif


#ifdef Part2
void main() {
    Uart_Init(115200);
    BOARD_Init();
    printf("Hello World\n");
    printf("S");
    
    while(1){
        unsigned char data = GetChar();
        if (data != 0){
            PutChar(data);
        }
    }
}
#endif
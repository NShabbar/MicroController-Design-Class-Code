#include "Uart.h"
#include "BOARD.h"
#include <xc.h>
#include <sys/attribs.h>

//#define FLAG

void UARTInit() {
    // Clear all Registers
    U1MODE = 0; // initialize Uart1 mode register to 0.
    //U2MODE = 0; // initialize Uart2 mode register to 0. 
    U1STA = 0; // initialize Uart1 status and control register to 0.
    //U2STA = 0; // intialize Uart2 status and control register to 0.
    U1TXREG = 0; // intialize Uart1 transmit register to 0.
    //U2TXREG = 0; // intialize Uart2 transmit register to 0.
    U1RXREG = 0; // intialize Uart1 receive register to 0.
    //U2RXREG = 0; // intialize Uart2 receive register to 0.

    //Calculate Baude Rate
    U1BRG = 21; // PBCLK = 40 MHz.
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
    // UxSTA is used because it is the status and control register.
    // It controls the status of the other two registers.
}

#ifndef Part1

void main() {
    BOARD_Init();
    UARTInit();
    while (1) {
        if (U1STAbits.URXDA == 1) {
            U1TXREG = U1RXREG;
        }
    }
}
#endif

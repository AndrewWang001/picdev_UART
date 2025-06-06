#include <pic18f26k83.h>

#include "uart.h"
#include "canlib.h"
#include "canlib/util/safe_ring_buffer.h"

// safe ring buffers for sending and receiving
static srb_ctx_t rx_buffer;
static srb_ctx_t tx_buffer;

// memory pools to use for those srbs. 100 is a completely arbitrary number
uint8_t rx_buffer_pool[100], tx_buffer_pool[100];

void uart_init(void)
{
    //--------------------------------------
    TRISB3 = 1;
    TRISB2 = 1;
    ANSELB3 = 0;
    ANSELB2 = 0;
    // set RX pin location
    U1RXPPS = (0b001 << 3) | // port B
              (0b011);       // pin 30
    // set CTS pin location
    U1CTSPPS = (0b001 << 3) | // port B
               (0b010);       // pin 2

    TRISB4 = 0;
    TRISB1 = 0;
    // Set B4 to TX
    RB4PPS = 0b010011; // UART1 TX
    // Set B1 to RTS
    RB1PPS = 0b010101; // UART1 RTS
    
    //--------------------------------------

    // enable flow control.
    U1CON2bits.FLO = 0b10;
    // low speed
    U1CON0bits.BRGS = 0;
    // don't autodetect baudrate
    U1CON0bits.ABDEN = 0;
    // normal mode (8 bit, no parity, no 9th bit)
    U1CON0bits.MODE = 0;
    // enable transmit
    U1CON0bits.TXEN = 1;
    // enable receive
    U1CON0bits.RXEN = 1;

    // keep running on overflow, never stop receiving
    U1CON2bits.RUNOVF = 1;

    // baud rate stuff, divide by 25 to get 115200 baud
    U1BRGL = 25;
    U1BRGH = 0;

    // we are go for UART
    U1CON1bits.ON = 1;

    // initialize the rx and tx buffers
    srb_init(&rx_buffer, rx_buffer_pool, sizeof(rx_buffer_pool), sizeof(uint8_t));
    srb_init(&tx_buffer, tx_buffer_pool, sizeof(tx_buffer_pool), sizeof(uint8_t));

    // enable receive interrupt
    IPR3bits.U1RXIP = 1;
    PIE3bits.U1RXIE = 1;
    // Do not enable transmit interrupt, that interrupt enable signals that
    // there is data to be sent, which at init time is not true
}

void uart_transmit_buffer(uint8_t *tx, uint8_t len)
{
    // just call uart_transmit_byte with each byte they want us to send
    while (len--)
    {
        srb_push(&tx_buffer, tx);
        tx++;
    }\
    // If the module isn't enabled, give it a byte to send and enable it
    if (PIE3bits.U1TXIE == 0)
    {
        srb_pop(&tx_buffer, &tx);
        U1TXB = tx;
        U1CON0bits.TXEN = 1;
        // also enable the interrupt for when it's ready to send more data
        PIE3bits.U1TXIE = 1;
    }
}

bool uart_byte_available(void)
{
    return !srb_is_empty(&rx_buffer);
}

uint8_t uart_read_byte(void)
{
    uint8_t rcv;
    srb_pop(&rx_buffer, &rcv);
    return rcv;
}

void uart_interrupt_handler(void)
{
    if (PIR3bits.U1TXIF)
    {
        // check if there are any bytes we still want to transmit
        if (!srb_is_empty(&tx_buffer))
        {
            // if so, transmit them
            uint8_t tx;
            srb_pop(&tx_buffer, &tx);
            U1TXB = tx;
        }
        else
        {
            // If we have no data to send, disable this interrupt
            PIE3bits.U1TXIE = 0;
            // if not, disable the TX part of the uart module so that TXIF
            // isn't triggered again and so that we reenable the module on
            // the next call to uart_transmit_byte
            U1CON0bits.TXEN = 0;
        }
        PIR3bits.U1TXIF = 0;
    }
    else if (PIR3bits.U1RXIF)
    {
        // we received a byte, need to push into RX buffer and return
        uint8_t rcv = U1RXB;
        srb_push(&rx_buffer, &rcv);
        PIR3bits.U1RXIF = 0;
    }
    else if (PIR3bits.U1EIF)
    {
        // Some sort of error, ignore for now (TODO?)
        PIR3bits.U1EIF = 0;
    }
    else if (PIR3bits.U1IF)
    {
        PIR3bits.U1IF = 0;
    }
}

#include <xc.h>
#include "canlib.h"

#include "device_config.h"
#include "platform.h"
#include "radio.h"
#include "uart.h"
#include "adc.h"

#define MAX_LOOP_TIME_DIFF_ms 500
#define MAX_SENSOR_TIME_DIFF_ms 5

static void can_msg_handler(const can_msg_t *msg); //hello branch

//memory pool for the CAN tx buffer
uint8_t tx_pool[100];
uint8_t rx_pool[100];

int main(void) {

    // init our millis() function
    timer0_init();

    // set up pins
    gpio_init();


    // Enable global interrupts
    INTCON0bits.GIE = 1;


    // loop timer
    uint32_t last_millis = millis();
    uint32_t last_sensor_millis = millis();

    bool heartbeat = false;

    while (1) {
        // clear watchdog timer
        CLRWDT();
  
        if (millis() - last_millis > MAX_LOOP_TIME_DIFF_ms) {
            // update our loop counter
            last_millis = millis();

            // visual heartbeat indicator
            BLUE_LED_SET(heartbeat);
            RED_LED_SET(heartbeat);
            WHITE_LED_SET(heartbeat);
            heartbeat = !heartbeat;
        }
    }
}


static void __interrupt() interrupt_handler(void) {

    if (PIE3bits.TMR0IE == 1 && PIR3bits.TMR0IF == 1) {
        timer0_handle_interrupt();
        PIR3bits.TMR0IF = 0;
    }
}

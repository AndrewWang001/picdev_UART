#include "platform.h"
#include <xc.h>

// whether the leds turn on when the pin is set to high or low
#define LED_ON 0

void gpio_init(void) {
    // set as outputs
    TRISB4 = 0; // LED
    TRISB3 = 0; // LED
    TRISB2 = 0; // LED
    // turn LEDs off
    LATB4 = !LED_ON;
    LATB3 = !LED_ON;
    LATB2 = !LED_ON;
    
    // current sense input
    TRISA4 = 1;
    // initially power off radio
    TRISC4 = 0;
    LATC4 = 0;
}

void RED_LED_SET(bool value) {
    LATB4 = !value ^ LED_ON;
}
void BLUE_LED_SET(bool value) {
    LATB3 = !value ^ LED_ON;
}
void WHITE_LED_SET(bool value) {
    LATB2 = !value ^ LED_ON;
}

void SET_RADIO_POWER(bool value) {
    LATC4 = value;
    RED_LED_SET(value);
}

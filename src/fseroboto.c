#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial.h"

int main() {

#define TOPCT2 255
#define RXPWM   (PD5)
#define PWM1OUT (PB1)
#define PWM2OUT (PB2)
#define RXPIN   ((1 << PD5) & PIND) >> PIND5

enum { LOW, HIGH };

void
ioinit () {
    DDRD = _BV(PD3);
    DDRB = _BV(PWM1OUT) | _BV(PWM2OUT);

    /* Enable Timer1 as fast PWM controlled by ICR1 */
    TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(COM1B1) | _BV(COM1B0) | _BV(WGM11);
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12); 
    ICR1 = TOPCT2;
    OCR1A = OCR1B = 0;

    TCCR2B = _BV(CS22);

    /* Enable external interrupt by pin change */
    EICRA  = _BV(ISC10);
    EIMSK  = _BV(INT1);

    /* Listening for toggle states on PD5 port */
    PCICR  = _BV(PCIE2);
    PCMSK2 = _BV(PCINT21);

    /* Init serial monitor registers */
    serial_init();

    /* Redirect stdin/stdiout to serial port */
    stdin  = &SERIALIN;
    stdout = &SERIALOUT;

    /* Enable global interrupt flag */
    sei();
}

int 
main () {
    /* I/O handler */
    ioinit();
    
    for (;;);
}

ISR (PCINT2_vect) {
    uint8_t state = RXPIN == 0 ? LOW : HIGH;
    uint16_t RFPWM;

    if (state == LOW) {
        RFPWM = (TCNT2) <= TOPCT2 ? TOPCT2 - TCNT2: 0;
        OCR1A = OCR1B = RFPWM;
    } else TCNT2 = 0;
}

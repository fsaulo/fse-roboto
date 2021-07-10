#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial.h"

int main() {

#define RX_PWM (PD5)
#define PWMOUT (PB1)
#define RXPIN  ((1 << PD5) & PIND) >> PIND5

enum { LOW, HIGH };

void
ioinit () {
    DDRD = _BV(PD3);
    DDRB = _BV(PB1);

    /* Enable Timer1 as fast PWM controlled by ICR1 */
    TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(WGM11);
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12); 
    ICR1 = 255;
    OCR1A = 0;

    TCCR2B = _BV(CS22);
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
        RFPWM = (TCNT2) <= 255 ? 255 - TCNT2: 0;
        OCR1A = RFPWM;
    } else TCNT2 = 0;
}

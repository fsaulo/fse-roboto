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
    TCCR2B = _BV(CS22);
    serial_init();

    stdin  = &SERIALIN;
    stdout = &SERIALOUT;

    printf("HELLO USART!\n");

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
    } else TCNT2 = 0;
}

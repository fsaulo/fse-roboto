#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial.h"

#define FILTER_LENGTH (5)
#define TOPCT2  (0x00FF)
#define TOPPOSC (0x029A)

#define PWM1OUT (PB1)
#define PWM2OUT (PB2)

#define RX2PIN  ((1 << PD2) & PIND) >> PIND2 == 0 ? LOW : HIGH
#define RX3PIN  ((1 << PD3) & PIND) >> PIND3 == 0 ? LOW : HIGH

enum { LOW, HIGH };

typedef struct {
    uint8_t CH2;
    uint8_t CH3;
} sync;

volatile sync RXSYNC;

volatile uint32_t POSCNT;
volatile uint32_t RF3PWM;
volatile uint32_t RF2CNT;
volatile uint32_t RF3CNT;

void
ioinit () {
    DDRB = _BV (PWM1OUT) | _BV (PWM2OUT);
    DDRD = _BV (PD6);
    DDRB = _BV (PB1);

    /* External interrupt */
    EICRA = _BV (ISC10) | _BV (ISC00);
    EIMSK = _BV (INT1) | _BV (INT0);

    /* Timer 2 */
    TCCR2A = _BV (COM2A0) | _BV (WGM21);
    TCCR2B = _BV (CS21);
    TIMSK2 = _BV (OCIE2A);
    OCR2A = 1;

    serial_init();

    stdin  = &SERIALIN;
    stdout = &SERIALOUT;

    POSCNT = 0;
    RF3PWM = 0;
    RF3CNT = 0;
    RF2CNT = 0;

    sei();
}

int 
main () {
    ioinit();

    for (;;); 
}

void
tune_in_ch1() {
    if (RX1PIN == LOW && RXSTR.CH1 == LOW) {
        /* No change, no operation*/
        _NOP();
    } else if ((RX1PIN == HIGH && RXSTR.CH1 == LOW) || RX1PIN == RXSTR.CH1) {
        /* Rising edge, do something [start counting] */
        RF1CNT += 2;
        RXSTR.CH1 = RX1PIN;
    } else if (RX1PIN != RXSTR.CH1) {
        /* Falling edge, stop counting, store value */
        RXSTR.CH1 = RX1PIN;
    }
}

ISR (TIMER2_COMPA_vect) {
    if (RX3PIN == HIGH && RXSYNC.CH3 == HIGH)
        RF3CNT++;
    else RXSYNC.CH3 = LOW;

    if (RX2PIN == HIGH && RXSYNC.CH2 == HIGH)
        RF2CNT++;
    else RXSYNC.CH2 = LOW;
}

ISR (INT0_vect) {
    RXSYNC.CH2 = HIGH;
    RF2CNT = 0;
}

ISR (INT1_vect) {
    RXSYNC.CH3 = HIGH;
    RF3CNT = 0;
}

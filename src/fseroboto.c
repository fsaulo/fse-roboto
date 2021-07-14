#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial.h"

#define OFFSET  (0x00A0)
#define TOPCT2  (0x00FF)
#define DTRES   (0x125)

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

    /* External interrupt: captures the incoming signal from the receiver */
    EICRA = _BV (ISC10) | _BV (ISC00);
    EIMSK = _BV (INT1) | _BV (INT0);

    /* Timer1: configured for fast PWM at undefined frequency */
    TCCR1A = _BV (COM1A1) | _BV (COM1B1) | _BV (WGM11);
    TCCR1B = _BV (WGM13) | _BV (WGM12) | _BV (CS11);
    ICR1 = DTRES;
    OCR1A = OCR1B = 0;

    /* Timer2: sets the speed direction by toggling OCRA/OCRB */
    TCCR2A = _BV (COM2A0) | _BV (WGM21);
    TCCR2B = _BV (CS21);
    TIMSK2 = _BV (OCIE2A);
    OCR2A = 1;

    /* Only used for debugging */
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
    TCCR1A &= 0b11;

    if (RF2CNT >= 300)
        TCCR1A |= _BV (COM1A1);
    else if (RF2CNT <= 200 && RF2CNT >= 100)
        TCCR1A |= _BV (COM1B1);

    RF2CNT = 0;
}

ISR (INT1_vect) {
    RXSYNC.CH3 = HIGH;
    OCR1A = (RF3CNT - OFFSET); 
    OCR1B = (RF3CNT - OFFSET); 
    RF3CNT = 0;
}

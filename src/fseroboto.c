#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial.h"

#define FILTER_LENGTH (5)
#define TOPCT2        (0x00FF)
#define TOPPOSC       (0x029A)

#define RXPWM   (PD5)
#define PWM1OUT (PB1)
#define PWM2OUT (PB2)

#define RX1PIN  ((1 << PD6) & PIND) >> PIND6 == 0 ? LOW : HIGH
#define RX2PIN  ((1 << PD3) & PIND) >> PIND3 == 0 ? LOW : HIGH
#define RX3PIN  ((1 << PD5) & PIND) >> PIND5 == 0 ? LOW : HIGH
#define RX4PIN  ((1 << PB3) & PIND) >> PINB3 == 0 ? LOW : HIGH


enum { LOW, HIGH };

typedef struct {
    uint8_t CH1;
    uint8_t CH2;
    uint8_t CH3;
    uint8_t CH4;
} receiver;

volatile receiver RXSTR;

volatile uint32_t POSCNT;
volatile uint32_t RF3PWM;
volatile uint32_t RF1CNT;
volatile uint32_t RF2CNT;
volatile uint32_t RF3CNT;

void
ioinit () {
    DDRB = _BV (PWM1OUT) | _BV (PWM2OUT);
    /* DDRD = _BV (PD7); */
    /* DDRB = _BV (PB0); */

    TCCR0A = _BV (WGM01);
    TCCR0B = _BV (CS00);

    OCR0A = 1;
    TIMSK0 = _BV (OCIE0A);

    /* Enable Timer1 as fast PWM controlled by ICR1 */
    TCCR1A = _BV (COM1A1) | _BV (COM1A0) | _BV (COM1B1) | _BV (COM1B0) | _BV (WGM11);
    TCCR1B = _BV (WGM13) | _BV (WGM12) | _BV (CS10); 

    ICR1 = TOPCT2;
    OCR1A = OCR1B = 0;

    TCCR2B = _BV (CS22);

    serial_init();

    stdin  = &SERIALIN;
    stdout = &SERIALOUT;

    POSCNT = 0;
    RF1CNT = 0;
    RF2CNT = 0;
    RF3PWM = 0;

    RXSTR.CH3 = HIGH;
    RXSTR.CH2 = HIGH;
    RXSTR.CH1 = HIGH;

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

void
tune_in_ch2() {
    if (RX2PIN == LOW && RXSTR.CH2 == LOW) {
        /* No change, no operation*/
        _NOP();
    } else if ((RX2PIN == HIGH && RXSTR.CH2 == LOW) || RX2PIN == RXSTR.CH2) {
        /* Rising edge, do something [start counting] */
        RF2CNT += 1;
        RXSTR.CH2 = RX2PIN;
    } else if (RX2PIN != RXSTR.CH2) {
        /* Falling edge, stop counting, store value */
        RXSTR.CH2 = RX2PIN;
    }
}

void
tune_in_ch3() {
    if (RX3PIN == LOW && RXSTR.CH3 == LOW) {
        /* No change, no operation*/
        _NOP();
    } else if ((RX3PIN == HIGH && RXSTR.CH3 == LOW) || RX3PIN == RXSTR.CH3) {
        /* Rising edge, do something [start counting] */
        RF3CNT += 2;
        RXSTR.CH3 = RX3PIN;
    } else if (RX3PIN != RXSTR.CH3) {
        /* Falling edge, stop counting, store value */
        RXSTR.CH3 = RX3PIN;
    }
}

ISR (TIMER0_COMPA_vect) {
    volatile uint8_t index = 0;
    volatile uint16_t sigch1aux_vect[FILTER_LENGTH];
    volatile uint16_t sigch2aux_vect[FILTER_LENGTH];
    volatile uint16_t sigch3aux_vect[FILTER_LENGTH];

    tune_in_ch1();
    tune_in_ch2();
    tune_in_ch3();
    
    if (POSCNT == TOPPOSC) {
        /* Here we perform a frequency division followed by a simple moving */
        /* average filtering */
        if (index < FILTER_LENGTH) {
            sigch3aux_vect[index] = RF3CNT;
            sigch2aux_vect[index] = RF2CNT;
            sigch1aux_vect[index] = RF1CNT;
            index++;
        } else {
            index = 0;
        }
        
        /* After filtering the variables are used to set the PWM duty cycle */
        /* This determines the output speed of the DC motors */
        for (index = 0; index < FILTER_LENGTH; index++) {
            RF1CNT += 0.9 * sigch1aux_vect[index];
            RF2CNT += 1.3 * sigch2aux_vect[index];
            RF3CNT += 2.4 * sigch3aux_vect[index];
        }

        RF1CNT = (210 - (RF1CNT / (FILTER_LENGTH+1))) & 0xFF; /* Full speed happens when RFCNT=0 */
        RF2CNT = ((RF2CNT / (FILTER_LENGTH+1))) & 0x1F4; /* Full speed happens when RFCNT=0 */
        RF3CNT = ~(140 - (RF3CNT / (FILTER_LENGTH+1))) & 0xFF; /* Full speed happens when RFCNT=0 */

        if (RF2CNT > 45 && RF2CNT < 60) {
            TCCR1A &= 0b11;
            PORTB &= ~(1 << PB1);
            PORTB &= ~(1 << PB2);
            OCR1A = OCR1B = 0;
        } else if (RF2CNT > 70) {
            TCCR1A &= 0b11;
            TCCR1A = _BV (COM1A0) | _BV (COM1A1);
            OCR1A = 0;
        } else if (RF2CNT < 30) {
            TCCR1A &= 0b11;
            TCCR1A = _BV (COM1B0) | _BV (COM1B1);
            OCR1A = 0;
        }

        OCR1A = RF3CNT;
        OCR1B = RF3CNT;


        /* End of clock cycle. Clear timing variable */
        POSCNT = RF3CNT = RF2CNT = 0;
    }
    POSCNT += 1;
}

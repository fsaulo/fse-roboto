#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define F_CPU 16000000UL
#define BAUD  9600

#include <util/delay.h>
#include <util/setbaud.h>


void serial_init() {
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
}

int serial_out(char data, FILE* stream) {
	if (data == '\n') {
		serial_out('\r', stream);
	}

	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
	return 0;
}

int serial_in(FILE* stream) {
	while(!(UCSR0A & (1 << RXC0)));
	return UDR0;
}

FILE serial_stdin  = FDEV_SETUP_STREAM(NULL, serial_in, _FDEV_SETUP_READ);
FILE serial_stdout = FDEV_SETUP_STREAM(serial_out, NULL, _FDEV_SETUP_WRITE);

int main() {

    serial_init();
    
    stdin  = &serial_stdin;
    stdout = &serial_stdout;

    printf("HELLO USART!\n");

    for (;;) {
        
    }
}

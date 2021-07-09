#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial.h"

int main() {

    serial_init();

    stdin  = &SERIALIN;
    stdout = &SERIALOUT;

    printf("HELLO USART!\n");

    for (;;) {
        
    }
}

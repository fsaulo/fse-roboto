#ifndef SERIAL_H
#define SERIAL_H
#endif

#define F_CPU 16000000UL
#define BAUD  9600

#include <stdio.h>
#include <util/delay.h>
#include <util/setbaud.h>

void serial_init();
int  serial_out(char data, FILE* stream);
int  serial_in(FILE* stream);

extern FILE SERIALIN;
extern FILE SERIALOUT;

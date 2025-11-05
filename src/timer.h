/*
 * Timer para contagem regressiva de 1 segundo
 */

#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>

void timer_init(void);
// timer_second_callback() deve ser implementada no main.c

#endif // TIMER_H


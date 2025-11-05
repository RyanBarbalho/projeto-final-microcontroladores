/*
 * Driver UART para comunicação serial
 * Baud rate configurável
 */

#ifndef UART_H
#define UART_H

#include <avr/io.h>
#include <stdint.h>

void uart_init(uint32_t baudrate);
void uart_send(uint8_t data);
uint8_t uart_receive(void);
uint8_t uart_available(void);
void uart_send_string(const char *str);

#endif // UART_H


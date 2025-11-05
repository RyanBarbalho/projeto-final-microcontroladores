/*
 * Implementação do driver UART
 */

#include "uart.h"
#include <util/delay.h>

void uart_init(uint32_t baudrate) {
    // Calcular UBRR para o baud rate desejado
    uint16_t ubrr = (F_CPU / 16 / baudrate) - 1;

    // Configurar UBRR
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;

    // Habilitar transmissor e receptor
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    // Configurar frame: 8 data bits, 1 stop bit, no parity
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_send(uint8_t data) {
    // Aguardar buffer de transmissão ficar vazio
    while (!(UCSR0A & (1 << UDRE0)));

    // Enviar dado
    UDR0 = data;
}

uint8_t uart_receive(void) {
    // Aguardar dado recebido
    while (!(UCSR0A & (1 << RXC0)));

    // Retornar dado
    return UDR0;
}

uint8_t uart_available(void) {
    // Verificar se há dado disponível
    return (UCSR0A & (1 << RXC0));
}

void uart_send_string(const char *str) {
    while (*str) {
        uart_send(*str++);
    }
}


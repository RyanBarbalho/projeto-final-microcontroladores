/*
 * Implementação do driver I2C
 * Para LCD com módulo I2C (PCF8574)
 */

#include "i2c.h"
#include <util/delay.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define I2C_FREQ 100000UL // 100kHz
#define I2C_TWBR_VAL (((F_CPU / I2C_FREQ) - 16) / 2)

void i2c_init(void) {
    // Configurar clock I2C
    TWBR = (uint8_t)I2C_TWBR_VAL;
    TWSR = 0; // Prescaler 1
    TWCR = (1 << TWEN); // Habilitar I2C
}

void i2c_start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

void i2c_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    while (TWCR & (1 << TWSTO));
}

void i2c_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

uint8_t i2c_read(uint8_t ack) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (ack ? (1 << TWEA) : 0);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}


/*
 * Driver I2C para LCD com módulo I2C (PCF8574)
 * Usado apenas se LCD_MODE_I2C estiver definido
 */

#ifndef I2C_H
#define I2C_H

#include <avr/io.h>
#include <stdint.h>

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_write(uint8_t data);
uint8_t i2c_read(uint8_t ack);

#endif // I2C_H


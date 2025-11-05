/*
 * Driver para LCD 16x2
 * Suporta modo paralelo (4-bit) ou I2C
 * Por padrão, usa modo I2C com PCF8574
 */

#ifndef LCD_H
#define LCD_H

#include <avr/io.h>
#include <util/delay.h>

// Configuração para modo I2C (PCF8574)
#define LCD_I2C_ADDRESS 0x27

// Configuração para modo paralelo (4-bit)
// Pinos: RS, E, D4, D5, D6, D7
#define LCD_RS_PORT    PORTD
#define LCD_RS_PIN     2
#define LCD_E_PORT     PORTD
#define LCD_E_PIN      3
#define LCD_D4_PORT    PORTD
#define LCD_D4_PIN     4
#define LCD_D5_PORT    PORTD
#define LCD_D5_PIN     5
#define LCD_D6_PORT    PORTD
#define LCD_D6_PIN     6
#define LCD_D7_PORT    PORTD
#define LCD_D7_PIN     7

// Modo de operação (descomente o modo desejado)
// #define LCD_MODE_I2C
#define LCD_MODE_PARALLEL

void lcd_init(void);
void lcd_command(uint8_t cmd);
void lcd_write(uint8_t data);
void lcd_clear(void);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_print(const char *str);
void lcd_print_char(char c);

#ifdef LCD_MODE_PARALLEL
void lcd_send_nibble(uint8_t nibble);
#endif

#ifdef LCD_MODE_I2C
#include "i2c.h"
void lcd_i2c_write(uint8_t data);
#endif

#endif // LCD_H


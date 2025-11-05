/*
 * Implementação do driver LCD 16x2
 */

#include "lcd.h"

#ifdef LCD_MODE_I2C
#include "i2c.h"
#endif

// Funções privadas
static void lcd_send_byte(uint8_t value, uint8_t mode);

#ifdef LCD_MODE_PARALLEL

void lcd_init(void) {
    // Configurar pinos como saída
    DDRD |= (1 << LCD_RS_PIN) | (1 << LCD_E_PIN) |
            (1 << LCD_D4_PIN) | (1 << LCD_D5_PIN) |
            (1 << LCD_D6_PIN) | (1 << LCD_D7_PIN);

    // Inicialização do LCD em modo 4-bit
    _delay_ms(50);

    // Sequência de inicialização
    lcd_send_nibble(0x03);
    _delay_ms(5);
    lcd_send_nibble(0x03);
    _delay_us(150);
    lcd_send_nibble(0x03);
    _delay_us(150);
    lcd_send_nibble(0x02); // Mudar para modo 4-bit
    _delay_us(150);

    // Configuração do LCD
    lcd_command(0x28); // 4-bit, 2 linhas, 5x8 font
    lcd_command(0x0C); // Display on, cursor off, blink off
    lcd_command(0x06); // Increment cursor, no shift
    lcd_command(0x01); // Clear display
    _delay_ms(2);
}

void lcd_send_nibble(uint8_t nibble) {
    // Enviar nibble alto (D7-D4)
    if (nibble & 0x01) LCD_D4_PORT |= (1 << LCD_D4_PIN);
    else LCD_D4_PORT &= ~(1 << LCD_D4_PIN);

    if (nibble & 0x02) LCD_D5_PORT |= (1 << LCD_D5_PIN);
    else LCD_D5_PORT &= ~(1 << LCD_D5_PIN);

    if (nibble & 0x04) LCD_D6_PORT |= (1 << LCD_D6_PIN);
    else LCD_D6_PORT &= ~(1 << LCD_D6_PIN);

    if (nibble & 0x08) LCD_D7_PORT |= (1 << LCD_D7_PIN);
    else LCD_D7_PORT &= ~(1 << LCD_D7_PIN);

    // Pulso Enable
    LCD_E_PORT |= (1 << LCD_E_PIN);
    _delay_us(1);
    LCD_E_PORT &= ~(1 << LCD_E_PIN);
    _delay_us(100);
}

void lcd_send_byte(uint8_t value, uint8_t mode) {
    // RS: 0 = comando, 1 = dado
    if (mode) LCD_RS_PORT |= (1 << LCD_RS_PIN);
    else LCD_RS_PORT &= ~(1 << LCD_RS_PIN);

    // Enviar nibble alto
    lcd_send_nibble(value >> 4);
    // Enviar nibble baixo
    lcd_send_nibble(value & 0x0F);
}

#else // LCD_MODE_I2C

void lcd_init(void) {
    i2c_init();
    _delay_ms(50);

    // Sequência de inicialização I2C
    lcd_i2c_write(0x03);
    _delay_ms(5);
    lcd_i2c_write(0x03);
    _delay_us(150);
    lcd_i2c_write(0x03);
    _delay_us(150);
    lcd_i2c_write(0x02);
    _delay_us(150);

    lcd_command(0x28);
    lcd_command(0x0C);
    lcd_command(0x06);
    lcd_command(0x01);
    _delay_ms(2);
}

void lcd_i2c_write(uint8_t data) {
    uint8_t nibble = (data & 0xF0) | 0x08; // Backlight on
    i2c_start();
    i2c_write(LCD_I2C_ADDRESS << 1);
    i2c_write(nibble);
    i2c_stop();
    _delay_us(1);
    i2c_start();
    i2c_write(LCD_I2C_ADDRESS << 1);
    i2c_write(nibble & ~0x08); // Enable low
    i2c_stop();
    _delay_us(100);
}

void lcd_send_byte(uint8_t value, uint8_t mode) {
    uint8_t high_nibble = (value & 0xF0) | 0x08 | (mode ? 0x04 : 0x00);
    uint8_t low_nibble = ((value << 4) & 0xF0) | 0x08 | (mode ? 0x04 : 0x00);

    i2c_start();
    i2c_write(LCD_I2C_ADDRESS << 1);
    i2c_write(high_nibble);
    i2c_stop();
    _delay_us(1);
    i2c_start();
    i2c_write(LCD_I2C_ADDRESS << 1);
    i2c_write(high_nibble & ~0x08);
    i2c_stop();
    _delay_us(100);

    i2c_start();
    i2c_write(LCD_I2C_ADDRESS << 1);
    i2c_write(low_nibble);
    i2c_stop();
    _delay_us(1);
    i2c_start();
    i2c_write(LCD_I2C_ADDRESS << 1);
    i2c_write(low_nibble & ~0x08);
    i2c_stop();
    _delay_us(100);
}

#endif

void lcd_command(uint8_t cmd) {
    lcd_send_byte(cmd, 0);
    _delay_us(200);
}

void lcd_write(uint8_t data) {
    lcd_send_byte(data, 1);
    _delay_us(50);
}

void lcd_clear(void) {
    lcd_command(0x01);
    _delay_ms(2);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    uint8_t address = (row == 0) ? 0x80 : 0xC0;
    address += col;
    lcd_command(address);
}

void lcd_print_char(char c) {
    lcd_write(c);
}

void lcd_print(const char *str) {
    while (*str) {
        lcd_write(*str++);
    }
}


/*
 * Driver PWM para controle de buzzer e LEDs
 * Suporta múltiplos canais PWM
 */

#ifndef PWM_H
#define PWM_H

#include <avr/io.h>
#include <stdint.h>

void pwm_init(void);
void pwm_set_duty(uint8_t pin, uint8_t duty);
void pwm_set_frequency(uint8_t pin, uint16_t frequency);

#endif // PWM_H


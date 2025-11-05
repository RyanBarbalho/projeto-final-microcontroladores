/*
 * Implementação do driver PWM
 */

#include "pwm.h"
#include <avr/interrupt.h>

// Timer 0 para PWM nos pinos 5 e 6 (LEDs)
// Timer 1 para PWM no pino 9 (Buzzer com frequência variável)
// Timer 2 usado para contagem regressiva (1 segundo)

void pwm_init(void) {
    // Configurar Timer 0 (PWM nos pinos 5 e 6 para LEDs)
    // Modo: Fast PWM, prescaler 64
    TCCR0A = (1 << WGM00) | (1 << WGM01) | (1 << COM0A1) | (1 << COM0B1);
    TCCR0B = (1 << CS01) | (1 << CS00); // Prescaler 64

    // Timer 1 será configurado dinamicamente para buzzer (pino 9)
    // Timer 2 será usado para contagem regressiva, não para PWM

    // Configurar pinos como saída
    DDRB |= (1 << PB1);  // Pin 9 (OC1A) - Buzzer
    DDRD |= (1 << PD5) | (1 << PD6); // Pins 5, 6 - LEDs
}

void pwm_set_duty(uint8_t pin, uint8_t duty) {
    switch (pin) {
        case 5:  // Timer 0, OC0B - LED vermelho
            OCR0B = duty;
            break;
        case 6:  // Timer 0, OC0A - LED amarelo
            OCR0A = duty;
            break;
        case 9:  // Timer 1, OC1A - Buzzer (frequência controlada por pwm_set_frequency)
            // Para buzzer, o duty cycle é controlado pela frequência
            // Se duty = 0, desligar buzzer
            if (duty == 0) {
                TCCR1A &= ~(1 << COM1A0);
                PORTB &= ~(1 << PB1);
            } else {
                // Reativar se necessário
                if (!(TCCR1A & (1 << COM1A0))) {
                    TCCR1A |= (1 << COM1A0);
                }
            }
            break;
        default:
            break;
    }
}

void pwm_set_frequency(uint8_t pin, uint16_t frequency) {
    // Para o buzzer no pino 9, vamos usar Timer 1 em modo CTC
    if (pin == 9) {
        // Calcular valor de comparação para a frequência desejada
        // F_CPU = 16MHz, prescaler = 64
        // frequency = F_CPU / (2 * prescaler * (1 + OCR1A))
        // OCR1A = (F_CPU / (2 * prescaler * frequency)) - 1

        if (frequency > 0 && frequency < 10000) {
            uint16_t ocr_value = (F_CPU / (2 * 64 * frequency)) - 1;
            if (ocr_value > 65535) ocr_value = 65535;
            if (ocr_value < 1) ocr_value = 1;

            // Modo CTC para gerar frequência precisa
            TCCR1A = 0;
            TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // CTC mode, prescaler 64
            OCR1A = ocr_value;
            TCCR1A |= (1 << COM1A0); // Toggle OC1A on compare match
        } else {
            // Desligar PWM
            TCCR1A &= ~(1 << COM1A0);
            PORTB &= ~(1 << PB1);
        }
    }
    // Para outros pinos, a frequência é fixa pelo prescaler
}


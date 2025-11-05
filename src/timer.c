/*
 * Implementação do timer para contagem regressiva
 * Usa Timer 2 em modo CTC para gerar interrupção a cada 1 segundo
 * (Timer 1 é usado para PWM do buzzer)
 */

#include "timer.h"

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// Contador de overflows para alcançar 1 segundo
static volatile uint8_t overflow_count = 0;

void timer_init(void) {
    // Usar Timer 2 para contagem regressiva (1 segundo)
    // Timer 2 é 8-bit, então usamos overflow counting
    // Prescaler: 1024
    // Frequência de overflow: 16000000 / (1024 * 256) ≈ 61.04 Hz
    // Para 1 segundo, precisamos de ~61 overflows

    // Modo: Normal (overflow)
    TCCR2A = 0;
    TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20); // Prescaler 1024

    // Habilitar interrupção de overflow
    TIMSK2 |= (1 << TOIE2);

    overflow_count = 0;
}

// Função callback (definida externamente no main.c)
extern void timer_second_callback(void);

// Interrupção de overflow do Timer 2
ISR(TIMER2_OVF_vect) {
    overflow_count++;
    // 61 overflows ≈ 1 segundo (16000000 / (1024 * 256) ≈ 61.04)
    if (overflow_count >= 61) {
        overflow_count = 0;
        // Chamar a função de callback
        timer_second_callback();
    }
}


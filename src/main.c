/*
 * Projeto Final - Simulador de Bomba
 * Arduino UNO ATmega328P
 *
 * Este projeto simula uma bomba com contagem regressiva,
 * feedback visual e sonoro, e sistema de desarme via UART.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "lcd.h"
#include "uart.h"
#include "pwm.h"
#include "timer.h"

// Estados do sistema
typedef enum {
    STATE_COUNTING,
    STATE_DISARMED,
    STATE_EXPLODED
} system_state_t;

// Configurações
#define INITIAL_TIME_SECONDS 60  // Tempo inicial em segundos
#define DISARM_CODE "1234"       // Código de desarme
#define BUZZER_PIN 9             // PWM pin para buzzer
#define LED_RED_PIN 5            // PWM pin para LED vermelho
#define LED_YELLOW_PIN 6         // PWM pin para LED amarelo

// Variáveis globais
volatile uint16_t remaining_time = INITIAL_TIME_SECONDS;
volatile system_state_t current_state = STATE_COUNTING;
volatile uint8_t time_updated = 0;
char received_code[10] = {0};
uint8_t code_index = 0;

// Protótipos
void system_init(void);
void update_display(void);
void update_feedback(void);
void check_serial_disarm(void);
void handle_explosion(void);
void handle_disarm(void);
uint16_t calculate_buzzer_frequency(uint16_t time_left);
uint8_t calculate_pwm_duty(uint16_t time_left);

int main(void) {
    system_init();

    char buffer[32];
    uint16_t minutes, seconds;

    sei(); // Habilitar interrupções globais

    // Mensagem inicial
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("BOMBA ATIVADA!");
    lcd_set_cursor(0, 1);
    lcd_print("TEMPO:");

    _delay_ms(2000);

    while (1) {
        if (current_state == STATE_COUNTING) {
            // Atualizar display
            update_display();

            // Atualizar feedback visual e sonoro
            update_feedback();

            // Verificar se chegou no desarme via serial
            check_serial_disarm();

            // Verificar se tempo acabou
            if (remaining_time == 0) {
                handle_explosion();
            }

            // Enviar código de desarme via serial periodicamente
            if (time_updated) {
                uart_send_string("CODE:");
                uart_send_string(DISARM_CODE);
                uart_send_string("\r\n");
                time_updated = 0;
            }

            _delay_ms(100);

        } else if (current_state == STATE_DISARMED) {
            // Bomba desarmada - manter estado
            _delay_ms(1000);

        } else if (current_state == STATE_EXPLODED) {
            // Bomba explodiu - manter estado
            _delay_ms(1000);
        }
    }

    return 0;
}

void system_init(void) {
    // Inicializar LCD
    lcd_init();

    // Inicializar UART
    uart_init(9600);

    // Inicializar PWM
    pwm_init();

    // Inicializar timer para contagem regressiva
    timer_init();

    // Configurar LEDs como saída
    DDRD |= (1 << LED_RED_PIN) | (1 << LED_YELLOW_PIN);
    DDRB |= (1 << BUZZER_PIN);
}

void update_display(void) {
    char buffer[17];
    uint16_t minutes = remaining_time / 60;
    uint16_t seconds = remaining_time % 60;

    lcd_set_cursor(0, 1);
    snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, seconds);
    lcd_print(buffer);
}

void update_feedback(void) {
    if (remaining_time == 0) {
        return; // Evitar cálculos quando tempo = 0
    }

    uint16_t frequency = calculate_buzzer_frequency(remaining_time);
    uint8_t duty = calculate_pwm_duty(remaining_time);

    // Atualizar buzzer com frequência variável
    // O buzzer faz beeps, e a frequência dos beeps aumenta conforme o tempo diminui
    static uint8_t beep_state = 0;
    static uint16_t beep_counter = 0;

    // Calcular intervalo entre beeps (em ciclos de ~100ms)
    // Quanto menor o tempo, menor o intervalo = mais beeps por segundo
    // Intervalo mínimo: 1 ciclo (beep contínuo quando tempo < 10s)
    // Intervalo máximo: 10 ciclos (beep a cada segundo quando tempo = 60s)
    uint16_t beep_interval;
    if (remaining_time <= 10) {
        beep_interval = 1; // Beep quase contínuo
    } else if (remaining_time <= 30) {
        // Intervalo aumenta linearmente de 1 a 5
        beep_interval = 1 + ((remaining_time - 10) * 4 / 20);
    } else {
        // Intervalo aumenta de 5 a 10
        beep_interval = 5 + ((remaining_time - 30) * 5 / 30);
    }

    beep_counter++;
    if (beep_counter >= beep_interval) {
        beep_state = !beep_state;
        beep_counter = 0;
    }

    if (beep_state) {
        pwm_set_frequency(BUZZER_PIN, frequency);
        pwm_set_duty(BUZZER_PIN, 128); // Duty cycle médio para o beep
    } else {
        pwm_set_duty(BUZZER_PIN, 0); // Silenciar entre beeps
    }

    // Atualizar LEDs com intensidade crescente
    pwm_set_duty(LED_RED_PIN, duty);

    // LED amarelo pisca mais rápido conforme o tempo diminui
    if (remaining_time < 30) {
        pwm_set_duty(LED_YELLOW_PIN, duty);
    } else {
        pwm_set_duty(LED_YELLOW_PIN, duty / 2);
    }
}

void check_serial_disarm(void) {
    if (uart_available()) {
        char c = uart_receive();

        // Receber código de desarme
        if (c == '\r' || c == '\n') {
            received_code[code_index] = '\0';

            if (strcmp(received_code, DISARM_CODE) == 0) {
                handle_disarm();
            }

            // Limpar buffer
            code_index = 0;
            memset(received_code, 0, sizeof(received_code));
        } else if (code_index < sizeof(received_code) - 1) {
            received_code[code_index++] = c;
        }
    }
}

void handle_explosion(void) {
    current_state = STATE_EXPLODED;

    // Parar PWM
    pwm_set_duty(BUZZER_PIN, 0);
    pwm_set_duty(LED_RED_PIN, 0);
    pwm_set_duty(LED_YELLOW_PIN, 0);

    // LED vermelho máximo
    PORTD |= (1 << LED_RED_PIN);

    // Buzzer contínuo em frequência alta
    pwm_set_frequency(BUZZER_PIN, 2000);
    pwm_set_duty(BUZZER_PIN, 128);

    // Exibir mensagem de explosão
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("*** EXPLODIU ***");
    lcd_set_cursor(0, 1);
    lcd_print("BOOM!");

    uart_send_string("EXPLODED!\r\n");
}

void handle_disarm(void) {
    current_state = STATE_DISARMED;

    // Parar todos os feedbacks
    pwm_set_duty(BUZZER_PIN, 0);
    pwm_set_duty(LED_RED_PIN, 0);
    pwm_set_duty(LED_YELLOW_PIN, 0);

    // Acender LED verde (se disponível) ou desligar LEDs
    PORTD &= ~((1 << LED_RED_PIN) | (1 << LED_YELLOW_PIN));

    // Exibir mensagem de desarme
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("BOMBA DESARMADA");
    lcd_set_cursor(0, 1);
    lcd_print("SUCESSO!");

    uart_send_string("DISARMED!\r\n");
}

uint16_t calculate_buzzer_frequency(uint16_t time_left) {
    // Frequência base: 500Hz
    // Frequência máxima: 2000Hz
    // Aumenta conforme o tempo diminui
    uint16_t base_freq = 500;
    uint16_t max_freq = 2000;
    uint16_t freq_range = max_freq - base_freq;

    // Frequência aumenta inversamente proporcional ao tempo restante
    uint16_t frequency = base_freq + (freq_range * (INITIAL_TIME_SECONDS - time_left) / INITIAL_TIME_SECONDS);

    return frequency;
}

uint8_t calculate_pwm_duty(uint16_t time_left) {
    // Duty cycle aumenta conforme o tempo diminui
    // Mínimo: 10% (quando tempo inicial)
    // Máximo: 100% (quando próximo de zero)
    uint8_t min_duty = 25;
    uint8_t max_duty = 255;
    uint8_t duty_range = max_duty - min_duty;

    uint8_t duty = min_duty + (duty_range * (INITIAL_TIME_SECONDS - time_left) / INITIAL_TIME_SECONDS);

    return duty;
}

// Callback do timer (chamado a cada 1 segundo)
void timer_second_callback(void) {
    if (current_state == STATE_COUNTING && remaining_time > 0) {
        remaining_time--;
        time_updated = 1;
    }
}


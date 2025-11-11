/*
 * Projeto Final - Simulador de Bomba
 * Arduino UNO - Versão Tinkercad
 *
 * Componentes:
 * - Arduino Uno R3
 * - LCD 16x2
 * - Potenciômetro 250kΩ (para contraste do LCD - V0)
 * - Resistor 220Ω (para backlight do LCD - opcional)
 * - Resistor 1kΩ (para buzzer - opcional, pode conectar direto)
 * - Piezo (Buzzer)
 *
 * INSTRUÇÕES DE CONEXÃO NO TINKERCAD:
 *
 * LCD 16x2:
 * - VSS (GND) -> GND do Arduino
 * - VDD (5V) -> 5V do Arduino
 * - V0 (Contraste) -> Meio do Potenciômetro 250kΩ
 *                     (Extremos do pot: um no 5V, outro no GND)
 * - RS -> Pin 12 do Arduino
 * - E  -> Pin 11 do Arduino
 * - D4 -> Pin 5 do Arduino
 * - D5 -> Pin 4 do Arduino
 * - D6 -> Pin 3 do Arduino
 * - D7 -> Pin 2 do Arduino
 * - R/W -> GND (sempre em modo escrita)
 * - A (Backlight +) -> 5V (pode usar resistor 220Ω ou conectar direto)
 * - K (Backlight -) -> GND
 *
 * Buzzer (Piezo):
 * - Terminal positivo (+) -> Pin 9 do Arduino
 *   (pode usar resistor 1kΩ em série ou conectar direto)
 * - Terminal negativo (-) -> GND do Arduino
 *
 * LED Vermelho:
 * - Terminal positivo (+) -> Pin 6 do Arduino (através de resistor 220Ω)
 *   NOTA: Pin 6 suporta PWM (~) para efeitos de brilho variável
 * - Terminal negativo (-) -> GND do Arduino
 *
 * LED Verde:
 * - Terminal positivo (+) -> Pin 10 do Arduino (através de resistor 220Ω)
 *   NOTA: Pin 10 suporta PWM (~) para efeitos de brilho variável
 * - Terminal negativo (-) -> GND do Arduino
 *
 * Serial Monitor:
 * - Use o Serial Monitor do Tinkercad (ícone no canto superior direito)
 * - Baud rate: 9600
 * - Digite "1234" e pressione Enter para desarmar
 */

 #include <LiquidCrystal.h>

 // Configuração do LCD (RS, E, D4, D5, D6, D7)
 LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

 // Estados do sistema
 enum SystemState {
   STATE_COUNTING,
   STATE_DISARMED,
   STATE_EXPLODED
 };

// Configurações
#define INITIAL_TIME_SECONDS 60  // Tempo inicial em segundos
#define DISARM_CODE "1234"       // Código de desarme
#define BUZZER_PIN 9             // Pin para buzzer
#define LED_RED_PIN 6            // Pin para LED vermelho (PWM)
#define LED_GREEN_PIN 10         // Pin para LED verde (PWM)
#define SERIAL_BAUD 9600         // Baud rate da serial
#define BUZZER_FREQUENCY 1000    // Frequência fixa do buzzer (Hz)
#define BEEP_DURATION 100        // Duração de cada beep em milissegundos
#define EXPLOSION_BUZZER_DURATION 3000  // Duração do buzzer após explosão (ms)
#define LED_FADE_SPEED 5         // Velocidade do fade (quanto maior, mais rápido)
#define BREATHING_CYCLE_MS 2000  // Ciclo completo de respiração em ms (2 segundos)

// Variáveis globais
volatile unsigned long remaining_time = INITIAL_TIME_SECONDS; // em segundos
SystemState current_state = STATE_COUNTING;
unsigned long last_second = 0;
unsigned long last_beep = 0;
unsigned long beep_start_time = 0;
bool beep_active = false;
String received_code = "";
unsigned long explosion_time = 0;  // Tempo em que a explosão aconteceu
unsigned long error_message_time = 0;  // Tempo em que a mensagem de erro foi exibida
bool showing_error = false;  // Flag para indicar se está mostrando mensagem de erro
int led_red_brightness = 0;  // Brilho atual do LED vermelho (0-255)
int led_green_brightness = 0;  // Brilho atual do LED verde (0-255)
unsigned long breathing_start = 0;  // Tempo de início do efeito de respiração

 void setup() {
   // Inicializar LCD
   lcd.begin(16, 2);

   // Inicializar Serial
   Serial.begin(SERIAL_BAUD);

   // Configurar buzzer como saída
   pinMode(BUZZER_PIN, OUTPUT);

   // Configurar LEDs como saída
   pinMode(LED_RED_PIN, OUTPUT);
   pinMode(LED_GREEN_PIN, OUTPUT);

   // Inicializar LEDs (vermelho e verde desligados)
   analogWrite(LED_RED_PIN, 0);
   analogWrite(LED_GREEN_PIN, 0);
   led_red_brightness = 0;
   led_green_brightness = 0;

   // Mensagem inicial
   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.print("BOMBA ATIVADA!");
   lcd.setCursor(0, 1);
   lcd.print("TEMPO:");

   // Inicializar tempo
   last_second = millis();

   delay(2000);
 }

 void loop() {
   unsigned long current_time = millis();

  if (current_state == STATE_COUNTING) {
    // Verificar se a mensagem de erro já passou (2 segundos)
    if (showing_error && (current_time - error_message_time >= 2000)) {
      showing_error = false;
      // Limpar LCD ao voltar ao display normal
      lcd.clear();
    }

    // Atualizar contagem regressiva (a cada 1 segundo)
    if (current_time - last_second >= 1000) {
      last_second = current_time;

      if (remaining_time > 0) {
        remaining_time--;
      } else {
        handle_explosion();
        return;
      }

      // Código de desarme removido - não enviar mais automaticamente
    }

    // Atualizar display (só se não estiver mostrando erro)
    if (!showing_error) {
      update_display();
    }

    // Atualizar feedback sonoro
    update_feedback();

    // Atualizar LED vermelho (pisca junto com o buzzer)
    update_led();

    // Verificar desarme via serial
    check_serial_disarm();

   } else if (current_state == STATE_DISARMED) {
     // Bomba desarmada - efeito de respiração no LED verde
     noTone(BUZZER_PIN);
     update_breathing_led();
     delay(10);  // Delay menor para animação mais suave

   } else if (current_state == STATE_EXPLODED) {
     // Bomba explodiu - verificar se já passaram 3 segundos
     if (current_time - explosion_time >= EXPLOSION_BUZZER_DURATION) {
       // Parar o buzzer após 3 segundos
       noTone(BUZZER_PIN);
       // Fade out do LED vermelho
       if (led_red_brightness > 0) {
         led_red_brightness -= LED_FADE_SPEED;
         if (led_red_brightness < 0) led_red_brightness = 0;
         analogWrite(LED_RED_PIN, led_red_brightness);
       }
       analogWrite(LED_GREEN_PIN, 0);
     } else {
       // Durante a explosão, LED vermelho em brilho máximo
       led_red_brightness = 255;
       analogWrite(LED_RED_PIN, led_red_brightness);
       analogWrite(LED_GREEN_PIN, 0);
     }
     delay(10);
   }
 }

void update_display() {
  unsigned long minutes = remaining_time / 60;
  unsigned long seconds = remaining_time % 60;

  // Limpar linha 0 completamente
  lcd.setCursor(0, 0);
  lcd.print("BOMBA ATIVADA!   "); // Espaços extras para limpar

  // Atualizar linha 1 com o tempo
  lcd.setCursor(0, 1);
  lcd.print("TEMPO: ");
  if (minutes < 10) lcd.print("0");
  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10) lcd.print("0");
  lcd.print(seconds);
  // Limpar caracteres extras (se houver)
  for (int i = 13; i < 16; i++) {
    lcd.setCursor(i, 1);
    lcd.print(" ");
  }
}

 void update_feedback() {
   if (remaining_time == 0) {
     return;
   }

   unsigned long current_time = millis();

   // Frequência fixa do buzzer (não muda)
   unsigned int frequency = BUZZER_FREQUENCY;

   // Calcular intervalo entre beeps (muda a cada 20 segundos)
   // O intervalo diminui (beeps mais frequentes) a cada 20 segundos
   unsigned long beep_interval = calculate_beep_interval(remaining_time);

   // Se o beep está ativo, verificar se já passou a duração
   if (beep_active) {
     if (current_time - beep_start_time >= BEEP_DURATION) {
       // Desligar o beep após a duração definida
       noTone(BUZZER_PIN);
       beep_active = false;
     }
   }

   // Verificar se é hora de iniciar um novo beep
   if (!beep_active && (current_time - last_beep >= beep_interval)) {
     // Iniciar novo beep
     tone(BUZZER_PIN, frequency);
     beep_active = true;
     beep_start_time = current_time;
     last_beep = current_time;
   }
 }

void update_led() {
   unsigned long current_time = millis();

   // Calcular intensidade base do LED vermelho baseada no tempo restante
   // Quanto menor o tempo, maior a intensidade (mais urgente)
   // Intensidade mínima: 50 (20%), máxima: 255 (100%)
   int base_brightness = 50 + ((INITIAL_TIME_SECONDS - remaining_time) * 205 / INITIAL_TIME_SECONDS);
   if (base_brightness > 255) base_brightness = 255;
   if (base_brightness < 50) base_brightness = 50;

   // Efeito de fade in/out durante os beeps
   if (beep_active) {
     // Fade in durante o beep
     unsigned long beep_elapsed = current_time - beep_start_time;
     if (beep_elapsed < BEEP_DURATION) {
       // Fade in suave: começa em 0 e vai até base_brightness
       int fade_target = base_brightness;
       int fade_progress = (beep_elapsed * 100) / BEEP_DURATION; // 0-100%
       led_red_brightness = (fade_target * fade_progress) / 100;
     } else {
       led_red_brightness = base_brightness;
     }
   } else {
     // Fade out suave entre beeps
     if (led_red_brightness > 0) {
       led_red_brightness -= LED_FADE_SPEED;
       if (led_red_brightness < 0) led_red_brightness = 0;
     }
   }

   // Aplicar brilho ao LED vermelho
   analogWrite(LED_RED_PIN, led_red_brightness);

   // LED verde sempre desligado durante a contagem
   analogWrite(LED_GREEN_PIN, 0);
   led_green_brightness = 0;
 }

void update_breathing_led() {
   // Efeito de respiração (breathing) no LED verde quando desarmado
   // O LED "respira" suavemente de baixo para alto brilho

   unsigned long current_time = millis();

   // Inicializar o tempo de início do breathing se ainda não foi definido
   if (breathing_start == 0) {
     breathing_start = current_time;
   }

   // Calcular posição no ciclo de respiração (0 a 2*PI)
   unsigned long cycle_time = (current_time - breathing_start) % BREATHING_CYCLE_MS;
   float cycle_position = (cycle_time * 2.0 * PI) / BREATHING_CYCLE_MS;

   // Função seno para criar efeito de respiração suave
   // Usar seno ao quadrado para um efeito mais suave (mais tempo no brilho alto)
   float sine_value = sin(cycle_position);
   float brightness_factor = (sine_value * sine_value); // 0.0 a 1.0

   // Brilho mínimo: 80 (31%), máximo: 255 (100%)
   led_green_brightness = 80 + (int)(brightness_factor * 175);

   // Aplicar brilho ao LED verde
   analogWrite(LED_GREEN_PIN, led_green_brightness);

   // LED vermelho desligado
   analogWrite(LED_RED_PIN, 0);
   led_red_brightness = 0;
 }

void check_serial_disarm() {
  // Verificar se há dados disponíveis na serial
  if (Serial.available() > 0) {
    // Usar readStringUntil que é mais confiável no Tinkercad
    // Lê a string completa até encontrar quebra de linha
    String input = Serial.readStringUntil('\n');

    // Limpar espaços e caracteres de controle
    input.trim();

    // Verificar se recebeu algum código
    if (input.length() > 0) {
      // Comparar com o código de desarme
      if (input.equals(DISARM_CODE)) {
        handle_disarm();
        return;
      } else {
        // Senha incorreta - mostrar mensagem no LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SENHA INCORRETA");
        lcd.setCursor(0, 1);
        lcd.print("TENTE NOVAMENTE");

        // Marcar que está mostrando erro e o tempo
        showing_error = true;
        error_message_time = millis();
      }
    }
  }
}

 void handle_explosion() {
   current_state = STATE_EXPLODED;

   // Marcar o tempo da explosão
   explosion_time = millis();

   // Parar beeps intermitentes
   noTone(BUZZER_PIN);
   delay(50);

   // Buzzer contínuo na mesma frequência (será desligado após 3 segundos no loop)
   tone(BUZZER_PIN, BUZZER_FREQUENCY);

   // LED vermelho em brilho máximo durante a explosão
   led_red_brightness = 255;
   analogWrite(LED_RED_PIN, led_red_brightness);
   analogWrite(LED_GREEN_PIN, 0);
   led_green_brightness = 0;

   // Exibir mensagem de explosão
   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.print("*** EXPLODIU ***");
   lcd.setCursor(0, 1);
   lcd.print("BOOM!");

   // Serial.println("EXPLODED!"); // Removido para limpar terminal
 }

 void handle_disarm() {
   current_state = STATE_DISARMED;

   // Parar buzzer
   noTone(BUZZER_PIN);

   // Resetar tempo de breathing para começar do início
   breathing_start = millis();

   // Fade out do LED vermelho e fade in do LED verde
   // O loop principal cuidará do efeito de respiração
   led_red_brightness = 0;
   led_green_brightness = 0;
   analogWrite(LED_RED_PIN, 0);
   analogWrite(LED_GREEN_PIN, 0);

   // Exibir mensagem de desarme
   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.print("BOMBA DESARMADA");
   lcd.setCursor(0, 1);
   lcd.print("SUCESSO!");

   // Serial.println("DISARMED!"); // Removido para limpar terminal
 }

unsigned long calculate_beep_interval(unsigned long time_left) {
  // Intervalo entre beeps muda a cada 20 segundos
  // Quanto menor o tempo, menor o intervalo (beeps mais frequentes)

  // Determinar em qual faixa de 20 segundos estamos
  // 60-41: faixa 3, 40-21: faixa 2, 20-1: faixa 1
  unsigned long faixa = (time_left + 19) / 20; // Arredonda para cima (faixas de 20 segundos)

  // Intervalos em milissegundos para cada faixa
  // Faixa 3 (60-41s): 1000ms (1 beep por segundo)
  // Faixa 2 (40-21s): 500ms (2 beeps por segundo)
  // Faixa 1 (20-1s): 250ms (4 beeps por segundo)
  unsigned long interval;

  if (faixa >= 3) {
    interval = 1000; // 60-41 segundos
  } else if (faixa == 2) {
    interval = 500;  // 40-21 segundos
  } else {
    interval = 250;  // 20-1 segundos
  }

  return interval;
}


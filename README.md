# Documentação Técnica - Simulador de Bomba com Arduino

## 1. Visão Geral do Projeto

Este projeto implementa um simulador de bomba com contagem regressiva usando Arduino Uno. O sistema possui três estados principais: contagem regressiva, desarmado e explodido. Durante a contagem, o sistema fornece feedback visual (LEDs) e sonoro (buzzer) que se intensificam conforme o tempo diminui, criando uma sensação crescente de urgência.

---

## 2. Componentes e Conexões

### 2.1 Componentes Utilizados
- **Arduino Uno R3**: Microcontrolador principal
- **LCD 16x2**: Display para mostrar informações
- **Buzzer Piezo**: Gerador de som para alertas
- **LED Vermelho**: Indicador visual de alerta (Pin 6 - PWM)
- **LED Verde**: Indicador visual de sucesso (Pin 10 - PWM)
- **Resistores 220Ω**: Proteção para LEDs
- **Potenciômetro 250kΩ**: Controle de contraste do LCD

### 2.2 Mapeamento de Pinos
- **Pins 2-5**: Dados do LCD (D7-D4)
- **Pin 6**: LED Vermelho (PWM)
- **Pin 9**: Buzzer
- **Pin 10**: LED Verde (PWM)
- **Pin 11**: Enable do LCD
- **Pin 12**: Register Select do LCD

---

## 3. Estrutura do Código

### 3.1 Bibliotecas e Configurações Iniciais

```cpp
#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
```

A biblioteca `LiquidCrystal` gerencia a comunicação com o display LCD. Os parâmetros definem os pinos de controle e dados.

### 3.2 Estados do Sistema

O sistema utiliza uma máquina de estados com três estados possíveis:

```cpp
enum SystemState {
  STATE_COUNTING,    // Bomba ativa, contando regressivamente
  STATE_DISARMED,    // Bomba desarmada com sucesso
  STATE_EXPLODED     // Tempo esgotado, bomba explodiu
};
```

**Fluxo de Estados:**
- Inicia em `STATE_COUNTING`
- Pode transicionar para `STATE_DISARMED` se o código correto for inserido
- Transiciona automaticamente para `STATE_EXPLODED` quando o tempo chega a zero

### 3.3 Constantes e Configurações

```cpp
#define INITIAL_TIME_SECONDS 60        // Tempo inicial: 60 segundos
#define DISARM_CODE "1234"            // Código de desarme
#define BUZZER_FREQUENCY 1000          // Frequência do buzzer: 1000 Hz
#define BEEP_DURATION 100              // Duração de cada beep: 100ms
#define LED_FADE_SPEED 5               // Velocidade do fade: 5 unidades/loop
#define BREATHING_CYCLE_MS 2000        // Ciclo de respiração: 2 segundos
```

---

## 4. Funções Principais

### 4.1 `setup()` - Inicialização

A função `setup()` executa uma única vez ao ligar o Arduino:

1. **Inicialização do LCD**: Configura o display 16x2
2. **Inicialização Serial**: Configura comunicação serial a 9600 baud
3. **Configuração de Pinos**: Define buzzer e LEDs como saídas
4. **Inicialização de LEDs**: Garante que ambos começam desligados
5. **Mensagem Inicial**: Exibe "BOMBA ATIVADA!" no LCD
6. **Inicialização de Tempo**: Marca o tempo inicial para contagem

### 4.2 `loop()` - Loop Principal

O `loop()` executa continuamente e gerencia os três estados:

#### Estado: STATE_COUNTING (Contagem Regressiva)

1. **Gerenciamento de Mensagens de Erro**:
   - Verifica se uma mensagem de erro está sendo exibida
   - Após 2 segundos, remove a mensagem e retorna ao display normal

2. **Contagem Regressiva**:
   - A cada 1000ms (1 segundo), decrementa `remaining_time`
   - Quando `remaining_time` chega a zero, chama `handle_explosion()`

3. **Atualização do Display**:
   - Chama `update_display()` para mostrar o tempo restante no formato MM:SS
   - Só atualiza se não estiver mostrando mensagem de erro

4. **Feedback Sonoro**:
   - Chama `update_feedback()` para controlar os beeps do buzzer

5. **Feedback Visual**:
   - Chama `update_led()` para controlar o LED vermelho com efeitos PWM

6. **Verificação de Desarme**:
   - Chama `check_serial_disarm()` para verificar se um código foi inserido

#### Estado: STATE_DISARMED (Desarmado)

- Para o buzzer completamente
- Ativa o efeito de respiração no LED verde através de `update_breathing_led()`
- Delay de 10ms para animação suave

#### Estado: STATE_EXPLODED (Explodido)

- Durante os primeiros 3 segundos:
  - Mantém buzzer contínuo em brilho máximo
  - LED vermelho em brilho máximo (255)
- Após 3 segundos:
  - Para o buzzer
  - Faz fade out gradual do LED vermelho
  - Desliga o LED verde

---

## 5. Funções de Atualização

### 5.1 `update_display()` - Atualização do LCD

**Funcionamento:**
1. Calcula minutos e segundos a partir do tempo restante
2. Formata o tempo no formato "TEMPO: MM:SS"
3. Adiciona zeros à esquerda quando necessário (ex: "05" em vez de "5")
4. Limpa caracteres extras na linha do tempo

**Exemplo de Saída:**
```
BOMBA ATIVADA!
TEMPO: 00:45
```

### 5.2 `update_feedback()` - Controle do Buzzer

**Funcionamento:**

1. **Cálculo do Intervalo entre Beeps**:
   - Chama `calculate_beep_interval()` para determinar o intervalo baseado no tempo restante
   - Intervalos variam conforme o tempo:
     - 60-41s: 1000ms (1 beep/segundo)
     - 40-21s: 500ms (2 beeps/segundo)
     - 20-11s: 250ms (4 beeps/segundo)
     - 10-1s: 200ms (5 beeps/segundo) - **Acelerado**

2. **Controle de Beeps**:
   - Se um beep está ativo e já passou `BEEP_DURATION` (100ms), desliga o buzzer
   - Se não há beep ativo e passou o intervalo calculado, inicia novo beep
   - Cada beep dura exatamente 100ms na frequência de 1000 Hz

**Efeito Psicológico:**
A aceleração progressiva dos beeps cria tensão crescente, simulando urgência real.

### 5.3 `update_led()` - Controle do LED Vermelho

**Funcionamento:**

1. **Cálculo de Brilho Base**:
   - Calcula intensidade base baseada no tempo restante
   - Fórmula: `base_brightness = 50 + ((60 - remaining_time) * 205 / 60)`
   - Resultado: varia de 50 (20%) a 255 (100%)
   - Quanto menor o tempo, maior o brilho

2. **Efeito de Fade In/Out**:
   - **Durante o beep** (beep_active = true):
     - Faz fade in suave durante os 100ms do beep
     - Progressão linear de 0 até `base_brightness`
   - **Entre beeps** (beep_active = false):
     - Faz fade out gradual subtraindo `LED_FADE_SPEED` (5 unidades)
     - Continua até chegar a 0

3. **Aplicação PWM**:
   - Usa `analogWrite()` para controlar o brilho via PWM
   - Valores de 0 (desligado) a 255 (brilho máximo)

**Efeito Visual:**
O LED vermelho pulsa suavemente junto com os beeps, aumentando de intensidade conforme o tempo diminui, criando um efeito visual dramático.

### 5.4 `update_breathing_led()` - Efeito de Respiração no LED Verde

**Funcionamento:**

1. **Inicialização**:
   - Se `breathing_start` é zero, inicializa com o tempo atual
   - Garante que o ciclo comece do início quando a bomba é desarmada

2. **Cálculo do Ciclo**:
   - Calcula posição no ciclo usando módulo: `(tempo_atual - inicio) % 2000ms`
   - Converte para radianos: `(ciclo * 2π) / 2000ms`
   - Usa função seno para criar onda suave

3. **Aplicação do Efeito**:
   - Eleva o seno ao quadrado para suavizar (mais tempo no brilho alto)
   - Mapeia para faixa de brilho: 80 (31%) a 255 (100%)
   - Fórmula: `brilho = 80 + (sen² * 175)`

4. **Aplicação PWM**:
   - Usa `analogWrite()` para aplicar o brilho calculado
   - Desliga o LED vermelho completamente

**Efeito Visual:**
O LED verde "respira" suavemente, criando um efeito calmante que indica sucesso no desarme.

---

## 6. Funções de Controle

### 6.1 `check_serial_disarm()` - Verificação de Código

**Funcionamento:**

1. **Leitura Serial**:
   - Verifica se há dados disponíveis na porta serial
   - Usa `readStringUntil('\n')` para ler até encontrar quebra de linha
   - Remove espaços com `trim()`

2. **Validação**:
   - Compara o código recebido com `DISARM_CODE` ("1234")
   - Se correto: chama `handle_disarm()`
   - Se incorreto: exibe mensagem de erro por 2 segundos

3. **Mensagem de Erro**:
   - Mostra "SENHA INCORRETA" e "TENTE NOVAMENTE"
   - Marca `showing_error = true` e registra o tempo
   - Após 2 segundos, retorna ao display normal

**Uso:**
No Serial Monitor do Tinkercad, digite "1234" e pressione Enter para desarmar.

### 6.2 `handle_explosion()` - Tratamento de Explosão

**Funcionamento:**

1. **Mudança de Estado**:
   - Altera `current_state` para `STATE_EXPLODED`
   - Registra o tempo da explosão em `explosion_time`

2. **Feedback Sonoro**:
   - Para beeps intermitentes
   - Inicia buzzer contínuo em 1000 Hz

3. **Feedback Visual**:
   - LED vermelho em brilho máximo (255)
   - LED verde desligado

4. **Mensagem no LCD**:
   - Exibe "*** EXPLODIU ***" na primeira linha
   - Exibe "BOOM!" na segunda linha

**Comportamento Posterior:**
O loop principal gerencia o fade out após 3 segundos.

### 6.3 `handle_disarm()` - Tratamento de Desarme

**Funcionamento:**

1. **Mudança de Estado**:
   - Altera `current_state` para `STATE_DISARMED`
   - Reseta `breathing_start` para iniciar o efeito de respiração

2. **Parada de Alertas**:
   - Para o buzzer completamente
   - Desliga ambos os LEDs inicialmente

3. **Mensagem no LCD**:
   - Exibe "BOMBA DESARMADA" na primeira linha
   - Exibe "SUCESSO!" na segunda linha

**Comportamento Posterior:**
O loop principal ativa o efeito de respiração no LED verde.

---

## 7. Funções Auxiliares

### 7.1 `calculate_beep_interval()` - Cálculo de Intervalo

**Funcionamento:**

Calcula o intervalo entre beeps baseado no tempo restante:

```cpp
if (time_left <= 10) {
  interval = 200;   // 5 beeps/segundo - Muito acelerado
} else if (time_left <= 20) {
  interval = 250;   // 4 beeps/segundo
} else if (time_left <= 40) {
  interval = 500;   // 2 beeps/segundo
} else {
  interval = 1000;  // 1 beep/segundo
}
```

**Lógica:**
- Quatro faixas de tempo com intervalos diferentes
- Quanto menor o tempo, menor o intervalo (beeps mais frequentes)
- Cria sensação crescente de urgência

---

## 8. Variáveis Globais Importantes

### 8.1 Controle de Tempo
- `remaining_time`: Tempo restante em segundos (volatile para uso em interrupções)
- `last_second`: Timestamp da última atualização de segundo
- `explosion_time`: Timestamp da explosão

### 8.2 Controle de Estados
- `current_state`: Estado atual do sistema (enum SystemState)
- `showing_error`: Flag indicando se mensagem de erro está sendo exibida
- `error_message_time`: Timestamp da mensagem de erro

### 8.3 Controle de Buzzer
- `beep_active`: Flag indicando se um beep está ativo
- `beep_start_time`: Timestamp do início do beep atual
- `last_beep`: Timestamp do último beep

### 8.4 Controle de LEDs
- `led_red_brightness`: Brilho atual do LED vermelho (0-255)
- `led_green_brightness`: Brilho atual do LED verde (0-255)
- `breathing_start`: Timestamp de início do efeito de respiração

---

## 9. Técnicas Utilizadas

### 9.1 PWM (Pulse Width Modulation)

**O que é:**
PWM não é um componente físico, mas uma técnica de controle já integrada no Arduino. Os pinos marcados com `~` suportam PWM.

**Como funciona:**
- O Arduino gera pulsos rápidos (liga/desliga)
- Variando a largura dos pulsos, controla-se a intensidade
- `analogWrite(pin, valor)` onde valor vai de 0 a 255

**Uso no Projeto:**
- Pin 6 (LED vermelho): Controla brilho variável e efeitos de fade
- Pin 10 (LED verde): Controla efeito de respiração

### 9.2 Máquina de Estados

O sistema utiliza uma máquina de estados simples para gerenciar os três estados possíveis. Cada estado tem comportamentos específicos no loop principal.

### 9.3 Non-Blocking Timing

O código usa `millis()` em vez de `delay()` para evitar bloqueios. Isso permite que múltiplas tarefas sejam executadas simultaneamente (LCD, buzzer, LEDs, serial).

**Exemplo:**
```cpp
if (current_time - last_second >= 1000) {
  // Executa a cada 1 segundo sem bloquear outras operações
}
```

### 9.4 Efeitos Visuais Avançados

- **Fade In/Out**: Transições suaves usando PWM
- **Breathing Effect**: Uso de função seno para criar onda suave
- **Brilho Progressivo**: Intensidade aumenta com urgência

---

## 10. Fluxo de Execução Completo

### 10.1 Inicialização (setup)
1. Configura LCD, Serial, pinos
2. Exibe mensagem inicial
3. Inicializa variáveis de tempo

### 10.2 Loop Principal (loop)
1. Lê tempo atual (`millis()`)
2. Verifica estado atual
3. Executa ações específicas do estado
4. Retorna ao início do loop

### 10.3 Estado de Contagem
1. Verifica mensagem de erro (2s timeout)
2. Decrementa tempo a cada 1 segundo
3. Atualiza display com tempo restante
4. Calcula e executa beeps do buzzer
5. Atualiza LED vermelho com fade
6. Verifica código de desarme via serial

### 10.4 Transição para Desarmado
1. Código correto recebido via serial
2. Para buzzer
3. Muda estado para DISARMED
4. Inicia efeito de respiração no LED verde

### 10.5 Transição para Explodido
1. Tempo chega a zero
2. Para beeps intermitentes
3. Inicia buzzer contínuo
4. LED vermelho em brilho máximo
5. Após 3 segundos, faz fade out

---

## 11. Características Especiais

### 11.1 Feedback Progressivo
- **Sonoro**: Beeps aceleram conforme tempo diminui
- **Visual**: Brilho do LED aumenta com urgência
- **Temporal**: Intervalos menores criam tensão crescente

### 11.2 Efeitos PWM
- **LED Vermelho**: Fade in/out sincronizado com beeps + brilho progressivo
- **LED Verde**: Efeito de respiração contínuo quando desarmado

### 11.3 Tratamento de Erros
- Mensagem de erro temporária (2 segundos)
- Retorno automático ao display normal
- Não interrompe a contagem regressiva

### 11.4 Interface Serial
- Comunicação a 9600 baud
- Leitura de strings completas até quebra de linha
- Validação de código de desarme

---

## 12. Considerações Técnicas

### 12.1 Uso de `volatile`
A variável `remaining_time` é declarada como `volatile` para garantir que seja atualizada corretamente mesmo em contextos de interrupção (embora não seja usado neste código, é uma boa prática).

### 12.2 Precisão de Tempo
O sistema usa `millis()` que retorna milissegundos desde o início. A contagem regressiva é precisa até o segundo, mas os efeitos visuais e sonoros operam em escala de milissegundos.

### 12.3 Gerenciamento de Memória
O código usa `String` para leitura serial, o que pode causar fragmentação de memória em execuções longas. Para produção, seria melhor usar arrays de char.

### 12.4 Compatibilidade Tinkercad
O código foi otimizado para funcionar no Tinkercad, usando `readStringUntil()` que é mais confiável na plataforma simulada.

---

## 13. Resumo Executivo

Este projeto implementa um simulador de bomba completo com:
- **Contagem regressiva** de 60 segundos
- **Feedback visual** progressivo (LEDs com PWM)
- **Feedback sonoro** acelerado (buzzer)
- **Sistema de desarme** via serial
- **Três estados** bem definidos
- **Efeitos visuais avançados** (fade, breathing)
- **Interface LCD** para informações

O código demonstra uso eficiente de recursos do Arduino, incluindo PWM, comunicação serial, controle de timing não-bloqueante e máquina de estados simples.


# Projeto Final - Simulador de Bomba

## Descrição

Este projeto simula uma bomba com contagem regressiva desenvolvida para Arduino UNO (ATmega328P). O sistema exibe o tempo restante em um display LCD, fornece feedback visual e sonoro através de LEDs e buzzer, e permite o desarme através de comunicação serial UART.

## Componentes Utilizados

- **Arduino UNO** (ATmega328P)
- **Display LCD 16x2** (interface paralela ou I2C)
- **2x LEDs** (vermelho e amarelo)
- **Buzzer** (piezo ou ativo)
- **Interface UART** (via USB do Arduino)
- **Fonte de alimentação 5V** (USB ou externa)
- **Jumpers e protoboard**

## Conexões

### LCD 16x2 (Modo Paralelo 4-bit)
- **VSS** → GND
- **VDD** → 5V
- **V0** → Potenciômetro 10kΩ (ajuste de contraste)
- **RS** → Pin 2 (Digital)
- **E** → Pin 3 (Digital)
- **D4** → Pin 4 (Digital)
- **D5** → Pin 5 (Digital) - **Nota: Conflito com LED! Ver abaixo**
- **D6** → Pin 6 (Digital) - **Nota: Conflito com LED! Ver abaixo**
- **D7** → Pin 7 (Digital)
- **A/K** → 5V (backlight)
- **K** → GND (backlight)

### LEDs e Buzzer
- **LED Vermelho** → Pin 5 (PWM) + Resistor 220Ω → GND
- **LED Amarelo** → Pin 6 (PWM) + Resistor 220Ω → GND
- **Buzzer** → Pin 9 (PWM) → GND

**Nota Importante:** Os pinos 5 e 6 são usados tanto para o LCD quanto para os LEDs. Se você usar LCD em modo paralelo, você terá um conflito. Duas soluções:

1. **Usar LCD I2C** (recomendado): Compre um módulo I2C para LCD e conecte:
   - **SDA** → Pin A4 (Analog)
   - **SCL** → Pin A5 (Analog)
   - **VCC** → 5V
   - **GND** → GND

2. **Usar pinos diferentes para LEDs**: Modifique o código para usar outros pinos PWM (3, 10, 11)

### Comunicação Serial
- Conecte o Arduino ao computador via USB
- A comunicação serial será feita através da porta USB (pinos 0 e 1)

## Funcionalidades

1. **Contagem Regressiva**: Inicia com 60 segundos e decrementa a cada segundo
2. **Display LCD**: Mostra o tempo restante no formato MM:SS
3. **Feedback Visual**: LEDs aumentam de intensidade conforme o tempo diminui
4. **Feedback Sonoro**: Buzzer aumenta de frequência conforme o tempo diminui
5. **Código de Desarme**: Envia periodicamente o código via serial (padrão: "1234")
6. **Sistema de Desarme**: Recebe código via serial para desarmar a bomba
7. **Estados Finais**:
   - **Desarmada**: Mensagem de sucesso no LCD
   - **Explodida**: Mensagem de explosão, LED vermelho máximo e buzzer contínuo

## Compilação e Upload

### Pré-requisitos

- **AVR-GCC** (toolchain para compilação)
- **avrdude** (para upload)
- **Make** (opcional, mas recomendado)

#### Windows
1. Instale [WinAVR](http://winavr.sourceforge.net/) ou
2. Instale [MSYS2](https://www.msys2.org/) e depois:
   ```bash
   pacman -S mingw-w64-x86_64-avr-gcc
   pacman -S mingw-w64-x86_64-avr-binutils
   pacman -S mingw-w64-x86_64-avr-libc
   pacman -S mingw-w64-x86_64-avrdude
   pacman -S make
   ```

#### Linux
```bash
sudo apt-get install gcc-avr avr-libc avrdude make
```

#### macOS
```bash
brew install avr-gcc avrdude
```

### Compilação

1. Clone ou baixe este repositório
2. Edite o `Makefile` e ajuste a porta serial (PORT) se necessário:
   ```makefile
   PORT = COM3    # Windows
   # PORT = /dev/ttyUSB0  # Linux
   # PORT = /dev/tty.usbserial-*  # macOS
   ```

3. Compile o projeto:
   ```bash
   make
   ```

4. Faça upload para o Arduino:
   ```bash
   make flash
   ```

   Ou compile e faça upload de uma vez:
   ```bash
   make all flash
   ```

### Compilação Manual (sem Makefile)

```bash
# Compilar
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os -Wall -std=c99 -c src/main.c -o build/main.o
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os -Wall -std=c99 -c src/lcd.c -o build/lcd.o
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os -Wall -std=c99 -c src/uart.c -o build/uart.o
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os -Wall -std=c99 -c src/pwm.c -o build/pwm.o
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os -Wall -std=c99 -c src/timer.c -o build/timer.o
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os -Wall -std=c99 -c src/i2c.c -o build/i2c.o

# Linkar
avr-gcc -mmcu=atmega328p build/*.o -o build/bomb_timer.elf

# Gerar HEX
avr-objcopy -O ihex -R .eeprom build/bomb_timer.elf build/bomb_timer.hex

# Upload
avrdude -p atmega328p -c arduino -P COM3 -b 115200 -U flash:w:build/bomb_timer.hex:i
```

## Configuração

### Modo do LCD

Edite `src/lcd.h` para escolher o modo:

```c
// Para LCD paralelo (4-bit):
#define LCD_MODE_PARALLEL

// Para LCD I2C:
// #define LCD_MODE_I2C
```

### Código de Desarme

Edite `src/main.c` para alterar o código:

```c
#define DISARM_CODE "1234"  // Altere para o código desejado
```

### Tempo Inicial

Edite `src/main.c` para alterar o tempo inicial:

```c
#define INITIAL_TIME_SECONDS 60  // Altere para o tempo desejado
```

## Uso

### Desarme via Serial

1. Conecte o Arduino ao computador
2. Abra um monitor serial (9600 baud):
   - Arduino IDE: Tools → Serial Monitor
   - PuTTY (Windows)
   - screen /dev/ttyUSB0 9600 (Linux)
   - minicom ou screen (macOS/Linux)

3. O sistema enviará periodicamente: `CODE:1234`
4. Para desarmar, envie o código seguido de Enter: `1234`

### Teste Rápido

1. Carregue o código no Arduino
2. O LCD mostrará "BOMBA ATIVADA!" por 2 segundos
3. A contagem regressiva começará
4. Os LEDs e buzzer começarão a funcionar
5. Observe a intensidade e frequência aumentarem conforme o tempo diminui
6. Envie o código "1234" via serial para desarmar
7. Ou deixe o tempo acabar para ver a explosão

## Estrutura do Projeto

```
projeto-final-microcontroladores/
├── src/
│   ├── main.c          # Arquivo principal
│   ├── lcd.h/c         # Driver do LCD
│   ├── uart.h/c        # Driver UART
│   ├── pwm.h/c         # Driver PWM (LEDs e Buzzer)
│   ├── timer.h/c       # Timer para contagem regressiva
│   └── i2c.h/c         # Driver I2C (opcional, para LCD I2C)
├── build/              # Arquivos compilados (gerado automaticamente)
├── Makefile            # Arquivo de build
└── README.md           # Este arquivo
```

## Características Técnicas

- **Microcontrolador**: ATmega328P
- **Clock**: 16 MHz
- **Timer 0**: PWM para LEDs (pinos 5 e 6)
- **Timer 1**: PWM para Buzzer com frequência variável (pino 9)
- **Timer 2**: Contagem regressiva (1 segundo)
- **UART**: 9600 baud
- **PWM**: 8-bit (0-255) para LEDs, frequência variável para buzzer

## Troubleshooting

### LCD não mostra nada
- Verifique as conexões
- Ajuste o potenciômetro de contraste (V0)
- Verifique se o backlight está conectado
- Teste com um código simples de LCD primeiro

### LEDs não acendem
- Verifique se os resistores estão conectados
- Verifique se os pinos estão corretos
- Verifique se há conflito com LCD (pinos 5 e 6)

### Buzzer não funciona
- Verifique a polaridade (se for buzzer ativo)
- Teste diretamente no pino 9
- Verifique se o buzzer está funcionando (teste com 5V)

### Serial não funciona
- Verifique a porta COM
- Verifique o baud rate (9600)
- Verifique se o driver USB está instalado
- Tente outra porta USB

### Compilação falha
- Verifique se o AVR-GCC está instalado
- Verifique se o PATH está configurado
- Verifique os erros de compilação

## Licença

Este projeto é para fins educacionais.

## Autor

Projeto desenvolvido para a disciplina de Microcontroladores.

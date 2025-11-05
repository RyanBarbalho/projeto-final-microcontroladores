# Makefile para projeto Arduino UNO (ATmega328P)
# Compilação com avr-gcc

# Configurações
MCU = atmega328p
F_CPU = 16000000UL
PROGRAMMER = arduino
PORT = COM3
BAUDRATE = 115200

# Diretórios
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# Arquivos fonte
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
HEADERS = $(wildcard $(SRC_DIR)/*.h)

# Nome do executável
TARGET = bomb_timer
HEX_FILE = $(BUILD_DIR)/$(TARGET).hex
ELF_FILE = $(BUILD_DIR)/$(TARGET).elf

# Compilador e flags
CC = avr-gcc
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -Wextra -std=c99
LDFLAGS = -mmcu=$(MCU)

# Objetos padrão
.PHONY: all clean flash size help

all: $(HEX_FILE)

# Criar diretórios
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compilar arquivos objeto
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Linkar executável
$(ELF_FILE): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

# Gerar arquivo HEX
$(HEX_FILE): $(ELF_FILE)
	avr-objcopy -O ihex -R .eeprom $< $@
	avr-size --format=avr --mcu=$(MCU) $<

# Flash no Arduino
flash: $(HEX_FILE)
	avrdude -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -b $(BAUDRATE) -U flash:w:$(HEX_FILE):i

# Limpar arquivos compilados
clean:
	rm -rf $(BUILD_DIR)

# Mostrar tamanho do código
size: $(ELF_FILE)
	avr-size --format=avr --mcu=$(MCU) $<

# Ajuda
help:
	@echo "Makefile para projeto Arduino UNO"
	@echo ""
	@echo "Comandos disponíveis:"
	@echo "  make all      - Compilar o projeto"
	@echo "  make flash    - Compilar e fazer upload para o Arduino"
	@echo "  make clean    - Limpar arquivos compilados"
	@echo "  make size     - Mostrar tamanho do código compilado"
	@echo "  make help     - Mostrar esta mensagem"
	@echo ""
	@echo "Configurações:"
	@echo "  MCU:          $(MCU)"
	@echo "  F_CPU:       $(F_CPU)"
	@echo "  PROGRAMMER:  $(PROGRAMMER)"
	@echo "  PORT:        $(PORT)"
	@echo ""
	@echo "Para alterar a porta, edite a variável PORT no Makefile"


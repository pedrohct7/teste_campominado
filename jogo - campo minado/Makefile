CC = gcc
CFLAGS = -Wall -Wextra -g3 -Iinclude

# Diretórios
SRC_DIR = src
OBJ_DIR = build
OUT_FILE = build/game

# Arquivos fonte e objetos
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Regra padrão
all: $(OUT_FILE)

# Linkagem final
$(OUT_FILE): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lncurses

# Compilação dos .c para .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza
clean:
	@rm -f $(OBJS)
	@rm -f $(OUT_FILE)

# Execução
run: all
	./$(OUT_FILE)

# Declara alvos que não são arquivos reais
.PHONY: all clean run
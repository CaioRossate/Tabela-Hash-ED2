CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99 -fstack-protector-all
INCLUDES = -I../include -I../unity

BUILD_DIR = build
SRC_DIR = .
TEST_DIR = ../test
UNITY_DIR = ../unity

TARGET_TEST    = teste_hashing
TARGET_SISTEMA = bitnopolis

# Objetos do sistema principal
OBJS_SISTEMA = $(BUILD_DIR)/hashing.o \
               $(BUILD_DIR)/geo.o \
               $(BUILD_DIR)/pessoa.o \
               $(BUILD_DIR)/qry.o \
               $(BUILD_DIR)/pm.o \
               $(BUILD_DIR)/main.o

# Objetos do teste unitário
OBJS_TESTE = $(BUILD_DIR)/unity.o \
             $(BUILD_DIR)/hashing.o \
             $(BUILD_DIR)/pessoa.o \
             $(BUILD_DIR)/geo.o \
             $(BUILD_DIR)/test_hashing.o

all: $(TARGET_SISTEMA) $(TARGET_TEST)

# Linkagem do sistema Bitnópolis
$(TARGET_SISTEMA): $(OBJS_SISTEMA)
	$(CC) $(CFLAGS) $^ -o $@

# Linkagem do executável de testes
$(TARGET_TEST): $(OBJS_TESTE)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Regra genérica: compila qualquer .c da pasta src/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Unity (pasta separada)
$(BUILD_DIR)/unity.o: $(UNITY_DIR)/unity.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Arquivo de testes (pasta separada)
$(BUILD_DIR)/test_hashing.o: $(TEST_DIR)/test_hashing.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Executa apenas os testes unitários
run_tests: $(TARGET_TEST)
	@./$(TARGET_TEST)

clean:
	rm -f $(TARGET_TEST) $(TARGET_SISTEMA)
	rm -rf $(BUILD_DIR)
	rm -f *.hf *.hfd

.PHONY: all clean run_tests
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99 -fstack-protector-all
INCLUDES = -I../include -I../unity

BUILD_DIR = build
SRC_DIR = .
TEST_DIR = ../test
UNITY_DIR = ../unity

# Executavel do Teste Unitário e do Sistema Real
TARGET_TEST = teste_hashing
TARGET_SISTEMA = bitnopolis

# Objetos do Sistema Real
OBJS_SISTEMA = $(BUILD_DIR)/hashing.o \
               $(BUILD_DIR)/geo.o \
               $(BUILD_DIR)/pessoa.o \
               $(BUILD_DIR)/qry.o \
               $(BUILD_DIR)/main.o

# Objetos do Teste Unitário
OBJS_TESTE = $(BUILD_DIR)/unity.o \
             $(BUILD_DIR)/hashing.o \
             $(BUILD_DIR)/test_hashing.o

all: $(TARGET_TEST) $(TARGET_SISTEMA)

# Linkagem do Sistema Bitnópolis
$(TARGET_SISTEMA): $(OBJS_SISTEMA)
	$(CC) $(CFLAGS) $^ -o $@

# Linkagem do Teste
$(TARGET_TEST): $(OBJS_TESTE)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Regra genérica para compilar arquivos .c da pasta atual
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compila Unity
$(BUILD_DIR)/unity.o: $(UNITY_DIR)/unity.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compila o arquivo de teste da outra pasta
$(BUILD_DIR)/test_hashing.o: $(TEST_DIR)/test_hashing.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET_TEST) $(TARGET_SISTEMA)
	rm -rf $(BUILD_DIR)
	rm -f *.hf *.hfd

run_tests: $(TARGET_TEST)
	@./$(TARGET_TEST)

.PHONY: clean all run_tests
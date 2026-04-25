#include "unity.h"
#include "hashing.h"
#include <string.h>


typedef struct __attribute__((packed)) {
    char id[20];
    int dado_extra;
    float medida;
} ItemTeste;

Hash h;

void setUp(void) {
    h = inicializarHash("teste_unidade.hf", 1, sizeof(ItemTeste));
}

void tearDown(void) {
    encerrarHash(h);
    remove("teste_unidade.hf");
}

void test_DeveInserirEBuscarRegistroCompleto(void) {
    ItemTeste original = {"cep123", 42, 3.14f};
    ItemTeste recuperado;
    
    bool inseriu = inserirHash(h, &original);
    TEST_ASSERT_TRUE_MESSAGE(inseriu, "A inserção do registro falhou");
    
    bool encontrou = buscarHash(h, "cep123", &recuperado);
    
    TEST_ASSERT_TRUE_MESSAGE(encontrou, "O registro deveria ter sido encontrado");
    TEST_ASSERT_EQUAL_STRING(original.id, recuperado.id);
    TEST_ASSERT_EQUAL_INT(original.dado_extra, recuperado.dado_extra);
    TEST_ASSERT_EQUAL_FLOAT(original.medida, recuperado.medida);
}

void test_DeveAtualizarRegistroExistente(void) {
    ItemTeste item = {"id_unico", 10, 1.0f};
    ItemTeste atualizado = {"id_unico", 20, 2.0f};
    ItemTeste resultado;

    inserirHash(h, &item);
    inserirHash(h, &atualizado);

    buscarHash(h, "id_unico", &resultado);
    TEST_ASSERT_EQUAL_INT(20, resultado.dado_extra);
}

void test_NaoDeveEncontrarChaveInexistente(void) {
    ItemTeste destino;
    bool encontrou = buscarHash(h, "nao_existe", &destino);
    TEST_ASSERT_FALSE(encontrou); 
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_DeveInserirEBuscarRegistroCompleto);
    RUN_TEST(test_DeveAtualizarRegistroExistente);
    RUN_TEST(test_NaoDeveEncontrarChaveInexistente);
    return UNITY_END();
}
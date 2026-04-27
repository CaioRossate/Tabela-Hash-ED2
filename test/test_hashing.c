#include "unity.h"
#include "hashing.h"
#include "pessoa.h"
#include "geo.h"
#include <string.h>
#include <stdlib.h>

// Struct generica
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

// Testes do módulo hashing

void test_DeveInserirEBuscarRegistroCompleto(void) {
    ItemTeste original = {"cep123", 42, 3.14f};
    ItemTeste recuperado;

    bool inseriu = inserirHash(h, &original);
    TEST_ASSERT_TRUE_MESSAGE(inseriu, "A insercao do registro falhou");

    bool encontrou = buscarHash(h, "cep123", &recuperado);
    TEST_ASSERT_TRUE_MESSAGE(encontrou, "O registro deveria ter sido encontrado");
    TEST_ASSERT_EQUAL_STRING(original.id, recuperado.id);
    TEST_ASSERT_EQUAL_INT(original.dado_extra, recuperado.dado_extra);
    TEST_ASSERT_EQUAL_FLOAT(original.medida, recuperado.medida);
}

void test_DeveAtualizarRegistroExistente(void) {
    ItemTeste item     = {"id_unico", 10, 1.0f};
    ItemTeste atualizado = {"id_unico", 20, 2.0f};
    ItemTeste resultado;

    inserirHash(h, &item);
    inserirHash(h, &atualizado);
    buscarHash(h, "id_unico", &resultado);

    TEST_ASSERT_EQUAL_INT(20, resultado.dado_extra);
    TEST_ASSERT_EQUAL_FLOAT(2.0f, resultado.medida);
}

void test_NaoDeveEncontrarChaveInexistente(void) {
    ItemTeste destino;
    bool encontrou = buscarHash(h, "nao_existe", &destino);
    TEST_ASSERT_FALSE(encontrou);
}

void test_DeveRemoverRegistro(void) {
    ItemTeste item = {"para_remover", 99, 9.9f};
    ItemTeste destino;

    inserirHash(h, &item);
    TEST_ASSERT_TRUE(buscarHash(h, "para_remover", &destino));

    bool removeu = removerHash(h, "para_remover");
    TEST_ASSERT_TRUE_MESSAGE(removeu, "A remocao deveria ter sucedido");

    bool encontrou = buscarHash(h, "para_remover", &destino);
    TEST_ASSERT_FALSE_MESSAGE(encontrou, "Registro removido nao deveria ser encontrado");
}

void test_RemoverChaveInexistenteFalha(void) {
    bool removeu = removerHash(h, "fantasma");
    TEST_ASSERT_FALSE_MESSAGE(removeu, "Remover chave inexistente deveria retornar false");
}

void test_DeveForcaSplitEManterDados(void) {
    char chave[20];
    for (int i = 0; i < 25; i++) {
        ItemTeste item;
        snprintf(item.id, sizeof(item.id), "chave_%02d", i);
        item.dado_extra = i * 10;
        item.medida = (float)i;
        bool ok = inserirHash(h, &item);
        TEST_ASSERT_TRUE_MESSAGE(ok, "Insercao durante stress test falhou");
    }

    for (int i = 0; i < 25; i++) {
        snprintf(chave, sizeof(chave), "chave_%02d", i);
        ItemTeste recuperado;
        bool achou = buscarHash(h, chave, &recuperado);
        TEST_ASSERT_TRUE_MESSAGE(achou, "Registro perdido apos split");
        TEST_ASSERT_EQUAL_INT(i * 10, recuperado.dado_extra);
    }
}

typedef struct { int contagem; } CtxContar;

void visita_contar(void* reg, void* ctx) {
    (void)reg;
    ((CtxContar*)ctx)->contagem++;
}

void test_PercorrerHashContaRegistros(void) {
    int N = 15;
    for (int i = 0; i < N; i++) {
        ItemTeste item;
        snprintf(item.id, sizeof(item.id), "reg_%02d", i);
        item.dado_extra = i;
        item.medida = 0.0f;
        inserirHash(h, &item);
    }

    CtxContar ctx = {0};
    percorrerHash(h, &ctx, visita_contar);
    TEST_ASSERT_EQUAL_INT_MESSAGE(N, ctx.contagem,
        "percorrerHash deveria visitar todos os registros exatamente uma vez");
}

// Testes do módulo pessoa

void test_CriarPessoa_DadosCorretos(void) {
    void* p = criarPessoa("123.456.789-00", "Joao", "Silva", 'M', "01/01/1990");
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_STRING("123.456.789-00", habitante_get_cpf(p));
    TEST_ASSERT_EQUAL_STRING("Joao",  habitante_get_nome(p));
    TEST_ASSERT_EQUAL_STRING("Silva", habitante_get_sobrenome(p));
    TEST_ASSERT_EQUAL_CHAR('M', habitante_get_sexo(p));
    destruirPessoa(p);
}

void test_PessoaNova_EhSemTeto(void) {
    void* p = criarPessoa("111.222.333-44", "Maria", "Costa", 'F', "15/06/2000");
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_FALSE_MESSAGE(ehMorador(p), "Pessoa recém-criada deveria ser sem-teto");
    destruirPessoa(p);
}

void test_SetEnderecoTornaMorador(void) {
    void* p = criarPessoa("555.666.777-88", "Carlos", "Souza", 'M', "20/03/1985");
    TEST_ASSERT_NOT_NULL(p);
    setEnderecoPessoa(p, "cep01", 'N', 50.0, "apto101");
    TEST_ASSERT_TRUE_MESSAGE(ehMorador(p), "Apos setEndereco deveria ser morador");
    TEST_ASSERT_EQUAL_STRING("cep01", habitante_get_cep(p));
    destruirPessoa(p);
}

void test_DesvincularEndereco_VirasSemTeto(void) {
    void* p = criarPessoa("999.888.777-66", "Ana", "Lima", 'F', "10/10/1995");
    setEnderecoPessoa(p, "cep02", 'S', 30.0, "casa");
    TEST_ASSERT_TRUE(ehMorador(p));
    desvincularEndereco(p);
    TEST_ASSERT_FALSE_MESSAGE(ehMorador(p), "Apos desvincular deveria ser sem-teto");
    TEST_ASSERT_NULL(habitante_get_cep(p));
    destruirPessoa(p);
}

// Testes do módulo geo

void test_CriarQuadra_DadosCorretos(void) {
    Quadra q = criarQuadra("cepABC", 10.0, 20.0, 100.0, 50.0, "black", "white", "1.0");
    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_STRING("cepABC", getQuadraCEP(q));
    TEST_ASSERT_EQUAL_FLOAT(10.0, getQuadraX(q));
    TEST_ASSERT_EQUAL_FLOAT(20.0, getQuadraY(q));
    TEST_ASSERT_EQUAL_FLOAT(100.0, getQuadraW(q));
    TEST_ASSERT_EQUAL_FLOAT(50.0,  getQuadraH(q));
    destruirQuadra(q);
}

void test_Quadra_NullRetornaValoresPadrao(void) {
    TEST_ASSERT_NULL(getQuadraCEP(NULL));
    TEST_ASSERT_EQUAL_FLOAT(0.0, getQuadraX(NULL));
    TEST_ASSERT_EQUAL_FLOAT(0.0, getQuadraY(NULL));
    TEST_ASSERT_EQUAL_FLOAT(0.0, getQuadraW(NULL));
    TEST_ASSERT_EQUAL_FLOAT(0.0, getQuadraH(NULL));
}


int main(void) {
    UNITY_BEGIN();

    // Hashing
    RUN_TEST(test_DeveInserirEBuscarRegistroCompleto);
    RUN_TEST(test_DeveAtualizarRegistroExistente);
    RUN_TEST(test_NaoDeveEncontrarChaveInexistente);
    RUN_TEST(test_DeveRemoverRegistro);
    RUN_TEST(test_RemoverChaveInexistenteFalha);
    RUN_TEST(test_DeveForcaSplitEManterDados);
    RUN_TEST(test_PercorrerHashContaRegistros);

    // Pessoa
    RUN_TEST(test_CriarPessoa_DadosCorretos);
    RUN_TEST(test_PessoaNova_EhSemTeto);
    RUN_TEST(test_SetEnderecoTornaMorador);
    RUN_TEST(test_DesvincularEndereco_VirasSemTeto);

    // Geo
    RUN_TEST(test_CriarQuadra_DadosCorretos);
    RUN_TEST(test_Quadra_NullRetornaValoresPadrao);

    return UNITY_END();
}
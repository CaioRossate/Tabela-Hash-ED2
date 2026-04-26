#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "pessoa.h"

typedef struct __attribute__((packed)) {
    char cep[16];
    char face;
    double numero;
    char complemento[20];
} endereco_t;

typedef struct __attribute__((packed)) {
    char cpf[16];
    char nome[20];
    char sobrenome[20];
    char sexo;
    char data_nascimento[12];
    bool sem_teto;
    endereco_t endereco;
} habitante_t;

size_t getPessoaSize() {
    return sizeof(habitante_t);
}

void* criarPessoa(char* cpf, char* nome, char* sobrenome, char sexo, char* nasc) {
    habitante_t *novo = calloc(1, sizeof(habitante_t));
    if (!novo) return NULL;

    strncpy(novo->cpf, cpf, 15); novo->cpf[15] = '\0';
    strncpy(novo->nome, nome, 19); novo->nome[19] = '\0';
    strncpy(novo->sobrenome, sobrenome, 19); novo->sobrenome[19] = '\0';
    strncpy(novo->data_nascimento, nasc, 11); novo->data_nascimento[11] = '\0';
    
    novo->sexo = sexo;
    novo->sem_teto = true;
    return (void*)novo;
}

void destruirPessoa(void* p) {
    if (p) free(p);
}

void setEnderecoPessoa(void* hab, char* cep, char face, double numero, char* complemento) {
    habitante_t* h = (habitante_t*)hab;
    h->sem_teto = false;
    strncpy(h->endereco.cep, cep, 15); h->endereco.cep[15] = '\0';
    strncpy(h->endereco.complemento, complemento, 19); h->endereco.complemento[19] = '\0';
    h->endereco.face = face;
    h->endereco.numero = numero;
}

void desvincularEndereco(void* hab) {
    if (hab) ((habitante_t*)hab)->sem_teto = true;
}

const char* habitante_get_cpf(const void* hab) {
    return (hab != NULL) ? ((habitante_t*)hab)->cpf : NULL;
}

const char* habitante_get_nome(const void* hab) {
    return (hab != NULL) ? ((habitante_t*)hab)->nome : NULL;
}

const char* habitante_get_sobrenome(const void* hab) {
    return (hab != NULL) ? ((habitante_t*)hab)->sobrenome : NULL;
}

char habitante_get_sexo(const void* hab) {
    return (hab != NULL) ? ((habitante_t*)hab)->sexo : '\0';
}

const char* habitante_get_cep(const void* hab) {
    habitante_t* h = (habitante_t*)hab;
    if (h == NULL || h->sem_teto) return NULL;
    return h->endereco.cep;
}

bool ehMorador(const void* hab) {
    return (hab != NULL) ? !((habitante_t*)hab)->sem_teto : false;
}

void habitante_print_info(FILE *txt, const void* hab) {
    habitante_t* h = (habitante_t*)hab;
    if (!h) return;
    fprintf(txt, "CPF: %s | Nome: %s %s | Sexo: %c | Nasc: %s\n", 
            h->cpf, h->nome, h->sobrenome, h->sexo, h->data_nascimento);
    
    if (!h->sem_teto) {
        fprintf(txt, "   Endereço: CEP %s, Face %c, Num %.0lf, Cmpl: %s\n",
                h->endereco.cep, h->endereco.face, h->endereco.numero, h->endereco.complemento);
    } else {
        fprintf(txt, "   Situação: Sem-teto\n");
    }
}

char habitante_get_face(const void* hab) {
    habitante_t* h = (habitante_t*)hab;
    return h->endereco.face;
}

double habitante_get_numero(const void* hab) {
    habitante_t* h = (habitante_t*)hab;
    return h->endereco.numero;
}
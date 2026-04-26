#ifndef PESSOA_H
#define PESSOA_H

#include <stdbool.h>
#include <stdio.h>

/**
 * @file pessoa.h
 * @brief Definições para manipulação de habitantes de Bitnópolis.
 */

typedef void* Pessoa;

/**
 * @brief Retorna o tamanho em bytes da estrutura de habitante.
 */
size_t getPessoaSize();

/**
 * @brief Cria uma instância de pessoa em memória RAM.
 * @param cpf O CPF da pessoa (chave).
 * @param nome O nome da pessoa.
 * @param sobrenome O sobrenome da pessoa.
 * @param sexo O sexo da pessoa ('M' ou 'F').
 * @param nasc A data de nascimento da pessoa (formato "DD/MM/AAAA").
 * @return Retorna um ponteiro (Pessoa) para a estrutura criada, ou NULL em caso de falha na alocação.
 */
Pessoa criarPessoa(char* cpf, char* nome, char* sobrenome, char sexo, char* nasc);

/**
 * @brief Vincula um endereço a uma pessoa existente.
 * @param p A pessoa a ser vinculada.
 * @param cep O CEP da quadra onde a pessoa mora.
 * @param face A face da quadra onde a pessoa mora ('N', 'S', 'L', 'O').
 * @param num O número da residência na face da quadra.
 * @param complemento Qualquer informação adicional sobre o endereço (ex: "apto 101").
 */
void setEnderecoPessoa(Pessoa p, char* cep, char face, double num, char* complemento);

/**
 * @brief Remove o vínculo de endereço da pessoa (torna sem-teto).
 * @param p A pessoa a ser desvinculada.
 */
void desvincularEndereco(Pessoa p);

/**
 * @brief Verifica se a pessoa possui um endereço vinculado.
 * @param p A pessoa a ser verificada.
 * @return true se a pessoa tem um endereço vinculado; false caso contrário.
 */
bool ehMorador(const void* p);

/**
 * @brief Libera a memória alocada para a pessoa.
 * @param p A pessoa a ser destruída.
 */
void destruirPessoa(Pessoa p);

/**
 * @brief Retorna o CPF da pessoa.
 * @param p A pessoa a ser consultada.
 * @return Um ponteiro para a string contendo o CPF da pessoa.
 */
const char* habitante_get_cpf(const void* p);

/**
 * @brief Retorna o nome da pessoa.
 * @param p A pessoa a ser consultada.
 * @return Um ponteiro para a string contendo o nome da pessoa.
 */
const char* habitante_get_nome(const void* p);

/**
 * @brief Retorna o sobrenome da pessoa.
 * @param p A pessoa a ser consultada.
 * @return Um ponteiro para a string contendo o sobrenome da pessoa.
 */
const char* habitante_get_sobrenome(const void* p);

/**
 * @brief Retorna o sexo da pessoa ('M'/'F').
 * @param p A pessoa a ser consultada.
 * @return O caractere representando o sexo da pessoa.
 */
char habitante_get_sexo(const void* p); 

/**
 * @brief Retorna o CEP onde a pessoa mora.
 * @param p A pessoa a ser consultada.
 * @return Um ponteiro para a string contendo o CEP da pessoa, ou NULL se sem endereço.
 */
const char* habitante_get_cep(const void* p);

/**
 * @brief Retorna a face da quadra onde a pessoa mora.
 * @param p A pessoa a ser consultada.
 * @return O caractere representando a face da quadra ('N', 'S', 'L', 'O'), ou '\0' se sem endereço.
 */
char habitante_get_face(const void* p);

/**
 * @brief Retorna o número do endereço da pessoa.
 * @param p A pessoa a ser consultada.
 * @return O número da residência na face da quadra.
 */
double habitante_get_numero(const void* p);

/**
 * @brief Imprime todos os dados da pessoa no arquivo TXT.
 * @param txt O arquivo de texto aberto para escrita.
 * @param h A pessoa cujos dados serão impressos.
 */
void habitante_print_info(FILE *txt, const void* h);

#endif
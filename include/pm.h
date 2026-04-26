#ifndef PM_H
#define PM_H

#include "hashing.h"
#include <stdbool.h>

/**
 * @file pm.h
 * @brief Processamento dos comandos do arquivo .pm (pessoas e moradores).
 */

/**
 * @brief Processa o arquivo .pm e popula o hash de pessoas.
 * Cada linha pode ser:
 *   p cpf nome sobrenome sexo nasc
 *   m cpf cep face num compl
 * @param path_pm Caminho para o arquivo .pm
 * @param hash_pessoas Hash de pessoas 
 * @return true se processou corretamente, false caso contrário
 */
bool processarArquivoPm(const char* path_pm, Hash hash_pessoas);

#endif

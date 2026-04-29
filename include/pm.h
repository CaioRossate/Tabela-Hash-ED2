#ifndef PM_H
#define PM_H

#include "hashing.h"
#include <stdbool.h>

/**
 * @file pm.h
 * @brief Processamento dos comandos do arquivo .pm (pessoas e moradores).
 * @details O arquivo .pm contém comandos para inserir pessoas e moradores, 
 * bem como consultas relacionadas. Este módulo é responsável por processar esses comandos, 
 * interagir com o Hash de pessoas e gerar as saídas necessárias.
 */

/**
 * @brief Processa o arquivo .pm e popula o hash de pessoas.
 * @param path_pm Caminho para o arquivo .pm
 * @param hash_pessoas Hash de pessoas 
 * @return true se processou corretamente, false caso contrário
 */
bool processarArquivoPm(const char* path_pm, Hash hash_pessoas);

#endif

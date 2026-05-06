#ifndef QRY_H
#define QRY_H

#include <stdio.h>
#include "hashing.h"

/**
 * @file qry.h
 * @brief Execução dos comandos de consulta e atualização definidos no arquivo .qry.
 */

/**
 * @brief Faz uma pré-leitura do .qry e coleta todos os CEPs removidos por "rq".
 * @details Usado pelo main para saber quais quadras não devem ser desenhadas no SVG.
 * @param path_qry  Caminho para o arquivo .qry.
 * @param ceps_out  Array de strings alocado pelo chamador (capacidade max_ceps).
 * @param max_ceps  Capacidade máxima do array.
 * @return Número de CEPs coletados.
 */
int coletarCepsRemovidos(const char* path_qry, char ceps_out[][20], int max_ceps);

/**
 * @brief Processa o arquivo .qry e executa os comandos de consulta/atualização.
 * @param path_qry Caminho para o arquivo .qry a ser processado.
 * @param h_q Hash de quadras.
 * @param h_p Hash de pessoas.
 * @param fTxt Arquivo para o relatório do comando.
 * @param fSvg Arquivo SVG para marcar as operações no mapa.
 */
void processarArquivoQry(const char* path_qry, Hash h_q, Hash h_p, FILE* fTxt, FILE* fSvg);

/**
 * @brief Remove uma quadra do sistema (Comando: rq).
 * @details Remove a quadra pelo CEP. Moradores vinculados tornam-se sem-tetos.
 * @param h_q Hash de quadras.
 * @param h_p Hash de pessoas.
 * @param cep CEP da quadra a ser removida.
 * @param txt Arquivo para o relatório do comando.
 * @param svg Arquivo SVG para marcar as operações no mapa (não usado para rq).
 */
void comando_rq(Hash h_q, Hash h_p, char* cep, FILE* txt, FILE* svg);

/**
 * @brief Calcula o número de moradores de uma quadra (Comando: Pq).
 * @details Conta quantos moradores existem em cada face da quadra e imprime um relatório. Se a quadra existir, marca os números no SVG.
 * @param h_q Hash de quadras.
 * @param h_p Hash de pessoas.
 * @param cep CEP da quadra a ser consultada.
 * @param txt Arquivo para o relatório do comando.
 * @param svg Arquivo SVG para marcar as operações no mapa (desenha números de moradores).
 */
void comando_Pq(Hash h_q, Hash h_p, char* cep, FILE* txt, FILE* svg);

/**
 * @brief Produz estatísticas gerais sobre os habitantes (Comando: censo).
 * @details Conta o total de habitantes, moradores, sem-tetos, homens e mulheres, e suas proporções. Imprime um relatório detalhado.
 * @param h_p Hash de pessoas.
 * @param txt Arquivo para o relatório do comando.
 */
void comando_censo(Hash h_p, FILE* txt);

/**
 * @brief Reporta dados detalhados de um habitante (Comando: h?).
 * @details Busca o habitante pelo CPF e imprime suas informações. Se não encontrado, informa que o CPF não existe.
 * @param h_p Hash de pessoas.
 * @param cpf CPF do habitante a ser consultado.
 * @param txt Arquivo para o relatório do comando.
 */
void comando_h_pergunta(Hash h_p, char* cpf, FILE* txt);

/**
 * @brief Registra o nascimento de uma pessoa (Comando: nasc).
 * @details Cria uma nova pessoa e a insere no hash.
 * @param h_p Hash de pessoas.
 * @param cpf CPF da pessoa a ser registrada.
 * @param nome Nome da pessoa.
 * @param sobrenome Sobrenome da pessoa.
 * @param sexo Sexo da pessoa.
 * @param nasc Data de nascimento da pessoa.
 */
void comando_nasc(Hash h_p, char* cpf, char* nome, char* sobrenome, char sexo, char* nasc);

/**
 * @brief Registra o falecimento de uma pessoa (Comando: rip).
 * @details Remove a pessoa do hash. Se ela tinha endereço, marca um "X" vermelho no SVG.
 * @param h_p Hash de pessoas.
 * @param h_q Hash de quadras.
 * @param cpf CPF da pessoa a ser registrada.
 * @param txt Arquivo para o relatório do comando.
 * @param svg Arquivo SVG para marcar as operações no mapa.
 */
void comando_rip(Hash h_p, Hash h_q, char* cpf, FILE* txt, FILE* svg);

/**
 * @brief Realiza a mudança de endereço de um morador (Comando: mud).
 * @details Atualiza o endereço do morador. Se o novo CEP for válido, marca a nova localização no SVG.
 * @param h_p Hash de pessoas.
 * @param h_q Hash de quadras.
 * @param cpf CPF do morador.
 * @param cep CEP da nova quadra.
 * @param face Face da nova quadra.
 * @param num Número da nova residência.
 * @param cmpl Complemento da nova residência.
 * @param txt Arquivo para o relatório do comando.
 * @param svg Arquivo SVG para marcar as operações no mapa.
 */
void comando_mud(Hash h_p, Hash h_q, char* cpf, char* cep, char face, double num, char* cmpl, FILE* txt, FILE* svg);

/**
 * @brief Registra o despejo de um morador (Comando: dspj).
 * @details O morador perde o endereço e torna-se sem-teto.
 * @param h_p Hash de pessoas.
 * @param h_q Hash de quadras.
 * @param cpf CPF do morador a ser despejado.
 * @param txt Arquivo para o relatório do comando.
 * @param svg Arquivo SVG para marcar as operações no mapa.
 */
void comando_dspj(Hash h_p, Hash h_q, char* cpf, FILE* txt, FILE* svg);

#endif
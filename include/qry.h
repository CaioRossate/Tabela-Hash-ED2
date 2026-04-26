#ifndef QRY_H
#define QRY_H

#include <stdio.h>
#include "hashing.h"

/**
 * @file qry.h
 * @brief Execução dos comandos de consulta e atualização definidos no arquivo .qry.
 */

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
 * @param cep CEP da quadra.
 * @param txt Arquivo para o relatório do comando.
 * @param svg Arquivo SVG para marcar a remoção no mapa.
 */
void comando_rq(Hash h_q, Hash h_p, char* cep, FILE* txt, FILE* svg);

/**
 * @brief Calcula o número de moradores de uma quadra (Comando: Pq).
 * @details Reporta o total por face e o total geral da quadra.
 * @param h_q Hash de quadras. 
 * @param h_p Hash de pessoas. 
 * @param cep CEP da quadra.
 * @param txt Arquivo para o relatório do comando.
 * @param svg Arquivo SVG para marcar a consulta no mapa.

 */
void comando_Pq(Hash h_q, Hash h_p, char* cep, FILE* txt, FILE* svg);

/**
 * @brief Produz estatísticas gerais sobre os habitantes (Comando: censo).
 * @details Reporta totais de homens/mulheres, moradores/sem-tetos e proporções.
 * @param h_p Hash de pessoas. 
 * @param txt Arquivo para o relatório estatístico.
 */
void comando_censo(Hash h_p, FILE* txt);

/**
 * @brief Reporta dados detalhados de um habitante (Comando: h?).
 * @details Se for morador, reporta também o endereço completo.
 * @param h_p Hash de pessoas. 
 * @param cpf CPF da pessoa. 
 * @param txt Arquivo de saída.
 */
void comando_h_pergunta(Hash h_p, char* cpf, FILE* txt);

/**
 * @brief Registra o nascimento de uma pessoa (Comando: nasc).
 * @details Insere um novo habitante no sistema com os dados fornecidos.
 * @param h_p Hash de pessoas. 
 * @param cpf CPF do recém-nascido.
 * @param nome Nome do recém-nascido.
 * @param sobrenome Sobrenome do recém-nascido.
 * @param sexo Sexo do recém-nascido.
 * @param nasc Data de nascimento do recém-nascido.
 */
void comando_nasc(Hash h_p, char* cpf, char* nome, char* sobrenome, char sexo, char* nasc);

/**
 * @brief Registra o falecimento de uma pessoa (Comando: rip).
 * @details Reporta dados e, se morador, marca o endereço com uma cruz no SVG.
 * @param h_p Hash de pessoas. 
 * @param h_q Hash de quadras.
 * @param cpf CPF do falecido.
 * @param txt Arquivo para o relatório do falecimento.
 * @param svg Arquivo SVG para marcar o falecimento no mapa.
 */
void comando_rip(Hash h_p, Hash h_q, char* cpf, FILE* txt, FILE* svg);

/**
 * @brief Realiza a mudança de endereço de um morador (Comando: mud).
 * @details Atualiza o endereço e marca o destino com um quadrado vermelho no SVG.
 * @param h_p Hash de pessoas. 
 * @param h_q Hash de quadras.
 * @param cpf CPF do morador.
 * @param cep Novo CEP da quadra de destino.
 * @param face Face do destino (N, S, L, O).
 * @param num Número do destino.
 * @param cmpl Complemento do destino.
 * @param txt Arquivo para o relatório da mudança.
 * @param svg Arquivo SVG para marcar a mudança no mapa.
 */
void comando_mud(Hash h_p, Hash h_q, char* cpf, char* cep, char face, double num, char* cmpl, FILE* txt, FILE* svg);

/**
 * @brief Registra o despejo de um morador (Comando: dspj).
 * @details Reporta dados do despejo e marca o local com um círculo preto no SVG.
 * @param h_p Hash de pessoas. 
 * @param h_q Hash de quadras.
 * @param cpf CPF do despejado.
 * @param txt Arquivo para o relatório do despejo.
 * @param svg Arquivo SVG para marcar o despejo no mapa.
 */
void comando_dspj(Hash h_p, Hash h_q, char* cpf, FILE* txt, FILE* svg);

#endif
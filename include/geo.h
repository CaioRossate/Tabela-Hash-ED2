#ifndef GEO_H
#define GEO_H

#include <stdbool.h>
#include <stdio.h>
#include "hashing.h"

/**
 * @file geo.h
 * @brief Gerenciamento de quadras e processamento do arquivo .geo.
 */

typedef void* Quadra;

/**
 * @brief Retorna o tamanho em bytes da struct quadra.
 */
size_t getQuadraSize();

/**
 * @brief Processa o arquivo .geo e popula o hash de quadras
 * 
 */
bool processarArquivoGeo(const char* path_geo, Hash hash_quadras);

/**
 * @brief Gera a representação SVG de todas as quadras armazenadas no Hash.
 * @param h_q O manipulador do Hashfile de quadras.
 * @param fSvg Ponteiro para o arquivo SVG aberto para escrita.
 */
void gerarCidadeSVG(Hash h_q, FILE* fSvg);

/**
 * @brief Cria uma instância de quadra em memória.
 * * @param cep O CEP da quadra (chave).
 * * @param x Coordenada X da quadra.
 * * @param y Coordenada Y da quadra.
 * * @param w Largura da quadra.
 * * @param h Altura da quadra.
 * * @param cor_b Cor da borda.
 * * @param cor_p Cor de preenchimento.
 * * @param sw Largura da borda.
 * @return Retorna um ponteiro (Quadra) para a estrutura criada, ou NULL em caso de falha na alocação.
 */
Quadra criarQuadra(char* cep, double x, double y, double w, double h, char* cor_b, char* cor_p, char* sw);

/**
 * @brief Cria uma instância de quadra em memória usando as cores padrão (preenchimento branco, borda preta, largura 1.0).
 * @param cfill Cor de preenchimento.
 * @param cstrk Cor da borda.
 * @param sw Espessura da borda.
 * @return Retorna um ponteiro (Quadra) para a estrutura criada, ou NULL
 */
Quadra comando_cq (char* cfill, char* cstrk, char* sw);

/**
 * @brief Libera a memória alocada para a quadra.
 * @param q A quadra a ser destruída.
 */
void destruirQuadra(Quadra q);

// --- GETTERS ---

/**
 * @brief Retorna o CEP da quadra informada.
 * @param q A quadra da qual se deseja obter o CEP.
 * @return O CEP da quadra.
 */
char* getQuadraCEP(Quadra q);

/**
 * @brief Retorna a coordenada X da quadra.
 * @param q A quadra da qual se deseja obter a coordenada X.
 * @return A coordenada X da quadra.
 */
double getQuadraX(Quadra q);

/**
 * @brief Retorna a coordenada Y da quadra.
 * @param q A quadra da qual se deseja obter a coordenada Y.
 * @return A coordenada Y da quadra.
 */
double getQuadraY(Quadra q);

/**
 * @brief Retorna a largura da quadra.
 * @param q A quadra da qual se deseja obter a largura.
 * @return A largura da quadra.
 */
double getQuadraW(Quadra q);

/**
 * @brief Retorna a altura da quadra.
 * @param q A quadra da qual se deseja obter a altura.
 * @return A altura da quadra.
 */
double getQuadraH(Quadra q);

/**
 * @brief Retorna a cor de preenchimento da quadra.
 * @param q A quadra da qual se deseja obter a cor.
 * @return Ponteiro para a string de cor de preenchimento.
 */
const char* getQuadraCorP(Quadra q);

/**
 * @brief Retorna a cor da borda da quadra.
 * @param q A quadra da qual se deseja obter a cor.
 * @return Ponteiro para a string de cor da borda.
 */
const char* getQuadraCorB(Quadra q);

/**
 * @brief Retorna a espessura da borda da quadra.
 * @param q A quadra da qual se deseja obter a espessura.
 * @return Ponteiro para a string de espessura da borda.
 */
const char* getQuadraSW(Quadra q);

/**
 * @brief Gera o SVG com todas as pessoas desenhadas nas quadras.
 * @param h_p Hash de pessoas.
 * @param h_q Hash de quadras.
 * @param fSvg Arquivo SVG para escrita.
 */
void gerarPessoasSVG(Hash h_p, Hash h_q, FILE* fSvg);

#endif
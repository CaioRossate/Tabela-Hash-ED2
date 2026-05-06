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
 * @brief Processa o arquivo .geo e popula o hash de quadras.
 */
bool processarArquivoGeo(const char* path_geo, Hash hash_quadras);

/**
 * @brief Calcula o bounding box de todas as quadras do hash.
 * @param h_q O manipulador do Hashfile de quadras.
 * @param vx  Saída: X mínimo com margem.
 * @param vy  Saída: Y mínimo com margem.
 * @param vw  Saída: largura total com margem.
 * @param vh  Saída: altura total com margem.
 */
void calcularBBoxCidade(Hash h_q, double* vx, double* vy, double* vw, double* vh);

/**
 * @brief Desenha as quadras no SVG, excluindo as de CEPs removidos.
 * @param h_q O manipulador do Hashfile de quadras.
 * @param fSvg Arquivo SVG aberto para escrita.
 * @param ceps_removidos Array de CEPs que não devem ser desenhados (podem ser NULL).
 * @param n_removidos   Quantidade de CEPs no array.
 */
void gerarCidadeSVG(Hash h_q, FILE* fSvg, char ceps_removidos[][20], int n_removidos);

/**
 * @brief Cria uma instância de quadra em memória.
 */
Quadra criarQuadra(char* cep, double x, double y, double w, double h, char* cor_b, char* cor_p, char* sw);

/**
 * @brief Retorna uma quadra-template com as cores e espessura fornecidas (comando cq).
 */
Quadra comando_cq(char* cfill, char* cstrk, char* sw);

/**
 * @brief Libera a memória alocada para a quadra.
 */
void destruirQuadra(Quadra q);

//Getters

size_t getQuadraSize();

/**
 * @brief Retorna o CEP da quadra.
 * @param q A quadra a ser consultada.
 * @return O CEP da quadra ou NULL se q for NULL.
 */
char* getQuadraCEP(Quadra q);

/**
 * @brief Retorna a coordenada X da quadra.
 * @param q A quadra a ser consultada.
 * @return A coordenada X da quadra ou 0 se q for NULL.
 */
double getQuadraX(Quadra q);

/**
 * @brief Retorna a coordenada Y da quadra.
 * @param q A quadra a ser consultada.
 * @return A coordenada Y da quadra ou 0 se q for NULL.
 */
double getQuadraY(Quadra q);

/**
 * @brief Retorna a largura da quadra.
 * @param q A quadra a ser consultada.
 * @return A largura da quadra ou 0 se q for NULL.
 */
double getQuadraW(Quadra q);

/**
 * @brief Retorna a altura da quadra.
 * @param q A quadra a ser consultada.
 * @return A altura da quadra ou 0 se q for NULL.
 */
double getQuadraH(Quadra q);

 /**
  * @brief Retorna a espessura da linha da quadra.
  * @param q A quadra a ser consultada.
  * @return A espessura da linha da quadra ou NULL se q for NULL.
  */
const char* getQuadraCorP(Quadra q);

/**
 * @brief Retorna a cor de contorno da quadra.
 * @param q A quadra a ser consultada.
 * @return A cor de contorno da quadra ou NULL se q for NULL.
 */
const char* getQuadraCorB(Quadra q);

/**
 * @brief Retorna a cor de preenchimento da quadra.
 * @param q A quadra a ser consultada.
 * @return A cor de preenchimento da quadra ou NULL se q for NULL.
 */
const char* getQuadraSW(Quadra q);

#endif
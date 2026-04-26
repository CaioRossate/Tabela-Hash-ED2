#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "geo.h"
#include "hashing.h"

typedef struct __attribute__((packed)) {
    char cep[20];
    double x, y, w, h;
    char cor_p[20];
    char cor_b[20];
    char sw[10];
} quadra_t;

// tamanho para o main
size_t getQuadraSize() {
    return sizeof(quadra_t);
}

// Getters
char* getQuadraCEP(Quadra q) { return (q != NULL) ? ((quadra_t*)q)->cep : NULL; }
double getQuadraX(Quadra q)  { return (q != NULL) ? ((quadra_t*)q)->x : 0; }
double getQuadraY(Quadra q)  { return (q != NULL) ? ((quadra_t*)q)->y : 0; }
double getQuadraW(Quadra q)  { return (q != NULL) ? ((quadra_t*)q)->w : 0; }
double getQuadraH(Quadra q)  { return (q != NULL) ? ((quadra_t*)q)->h : 0; }

// Getters para as cores (públicos)
const char* getQuadraCorP(Quadra q) { return (q != NULL) ? ((quadra_t*)q)->cor_p : NULL; }
const char* getQuadraCorB(Quadra q) { return (q != NULL) ? ((quadra_t*)q)->cor_b : NULL; }
const char* getQuadraSW(Quadra q)   { return (q != NULL) ? ((quadra_t*)q)->sw : NULL; }

// Quadra
Quadra criarQuadra(char* cep, double x, double y, double w, double h, char* cor_b, char* cor_p, char* sw) {
    quadra_t* q = (quadra_t*) calloc(1, sizeof(quadra_t));
    if (q) {
        strncpy(q->cep, cep, 19);
        q->x = x; q->y = y; q->w = w; q->h = h;
        strncpy(q->cor_b, cor_b, 19);
        strncpy(q->cor_p, cor_p, 19);
        strncpy(q->sw, sw, 9);
    }
    return (Quadra)q;
}

void destruirQuadra(Quadra q) {
    if (q) free(q);
}

// Processamento do arquivo .geo
bool processarArquivoGeo(const char* path_geo, Hash hash_quadras) {
    FILE* arquivo = fopen(path_geo, "r");
    if (!arquivo) return false;

    char comando[10];
    char cfill[20] = "white";
    char cstrk[20] = "black";
    char sw[10] = "1.0";

    while (fscanf(arquivo, "%s", comando) != EOF) {
        if (strcmp(comando, "cq") == 0) {
            fscanf(arquivo, "%s %s %s", sw, cfill, cstrk);
        } 
        else if (strcmp(comando, "q") == 0) {
            char cep[20];
            double x, y, w, h;
            fscanf(arquivo, "%s %lf %lf %lf %lf", cep, &x, &y, &w, &h);

            Quadra nova = criarQuadra(cep, x, y, w, h, cstrk, cfill, sw);
            inserirHash(hash_quadras, nova); 
            destruirQuadra(nova); 
        }
    }
    fclose(arquivo);
    return true;
}

// SVG

void desenharQuadraSVG(void* reg, void* ctx) {
    FILE* svg = (FILE*) ctx;
    if (!reg || !svg) return;
    
    // Desenha o retângulo da quadra
    fprintf(svg, "\t<rect x=\"%lf\" y=\"%lf\" width=\"%lf\" height=\"%lf\" fill=\"%s\" stroke=\"%s\" stroke-width=\"%s\" />\n",
            getQuadraX(reg), 
            getQuadraY(reg), 
            getQuadraW(reg), 
            getQuadraH(reg),
            getQuadraCorP(reg),
            getQuadraCorB(reg),
            getQuadraSW(reg));
    
    // Desenha o texto com o CEP
    fprintf(svg, "\t<text x=\"%lf\" y=\"%lf\" font-size=\"4\" fill=\"black\">%s</text>\n", 
            getQuadraX(reg) + 2, getQuadraY(reg) + 10, getQuadraCEP(reg));
}

void gerarCidadeSVG(Hash h_q, FILE* fSvg) {
    if (!h_q || !fSvg) return;
    percorrerHash(h_q, fSvg, desenharQuadraSVG);
}
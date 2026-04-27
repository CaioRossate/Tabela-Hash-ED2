#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashing.h"
#include "geo.h"
#include "qry.h"
#include "pessoa.h"
#include "pm.h"

typedef enum { ENTRADA, SAIDA, GEO, QUERY, PM } FilePaths;

char* concatenarCaminho(const char* dir, const char* arquivo);

int main(int argc, char* argv[]) {

    char* flags[] = {"-e", "-o", "-f", "-q", "-pm"};
    char** paths = calloc(5, sizeof(char*));

    for (int i = 1; i < argc; i += 2) {
        for (int j = 0; j < 5; j++) {
            if (i + 1 < argc && strcmp(argv[i], flags[j]) == 0) {
                paths[j] = argv[i + 1];
            }
        }
    }

    if (!paths[SAIDA] || !paths[GEO]) {
        printf("Erro: Argumentos -f (geo) e -o (saida) sao obrigatorios.\n");
        free(paths);
        return 1;
    }

    Hash hashQuadras = inicializarHash("quadras.hf", 5, getQuadraSize());
    Hash hashPessoas = inicializarHash("pessoas.hf", 5, getPessoaSize());

    if (!hashQuadras || !hashPessoas) {
        printf("Erro ao inicializar arquivos de Hash.\n");
        free(paths);
        return 1;
    }

    // Arquivo .geo
    char* pathCompletoGeo = concatenarCaminho(paths[ENTRADA], paths[GEO]);
    processarArquivoGeo(pathCompletoGeo, hashQuadras);

    // Arquivo .pm: usa -pm se fornecido, senão deriva do .geo
    char* pathCompletoPm = NULL;
    if (paths[PM]) {
        pathCompletoPm = concatenarCaminho(paths[ENTRADA], paths[PM]);
    } else {
        // troca extensão do .geo por .pm
        char pathPmBuf[512];
        strncpy(pathPmBuf, pathCompletoGeo, sizeof(pathPmBuf) - 1);
        pathPmBuf[sizeof(pathPmBuf) - 1] = '\0';
        char* dot = strrchr(pathPmBuf, '.');
        if (dot) strcpy(dot, ".pm");
        pathCompletoPm = malloc(strlen(pathPmBuf) + 1);
        strcpy(pathCompletoPm, pathPmBuf);
    }
    processarArquivoPm(pathCompletoPm, hashPessoas);

    // Arquivo .qry
    if (paths[QUERY]) {
        char* pathCompletoQry = concatenarCaminho(paths[ENTRADA], paths[QUERY]);

        // Monta nomes dos arquivos de saída: <geoBase>-<qryBase>.txt/.svg
        char geoBase[128];
        strncpy(geoBase, paths[GEO], sizeof(geoBase) - 1);
        geoBase[sizeof(geoBase) - 1] = '\0';
        char* dotGeo = strrchr(geoBase, '.');
        if (dotGeo) *dotGeo = '\0';

        char qryBase[128];
        strncpy(qryBase, paths[QUERY], sizeof(qryBase) - 1);
        qryBase[sizeof(qryBase) - 1] = '\0';
        char* barra = strrchr(qryBase, '/');
        if (barra) memmove(qryBase, barra + 1, strlen(barra));

        char pathSaidaTxt[512], pathSaidaSvg[512];
        snprintf(pathSaidaTxt, sizeof(pathSaidaTxt), "%s/%s-%s.txt", paths[SAIDA], geoBase, qryBase);
        snprintf(pathSaidaSvg, sizeof(pathSaidaSvg), "%s/%s-%s.svg", paths[SAIDA], geoBase, qryBase);

        FILE* fTxt = fopen(pathSaidaTxt, "w");
        FILE* fSvg = fopen(pathSaidaSvg, "w");

        if (!fTxt) { printf("Erro ao criar TXT: %s\n", pathSaidaTxt); }
        if (!fSvg) { printf("Erro ao criar SVG: %s\n", pathSaidaSvg); }

        if (fTxt && fSvg) {
            fprintf(fSvg, "<svg xmlns=\"http://www.w3.org/2000/svg\">\n");
            gerarCidadeSVG(hashQuadras, fSvg);
            processarArquivoQry(pathCompletoQry, hashQuadras, hashPessoas, fTxt, fSvg);
            fprintf(fSvg, "</svg>\n");
            fclose(fTxt);
            fclose(fSvg);
        } else {
            if (fTxt) fclose(fTxt);
            if (fSvg) fclose(fSvg);
        }

        free(pathCompletoQry);
    }

    gerarRelatorioHash(hashQuadras, "relatorio_quadras.hfd");
    gerarRelatorioHash(hashPessoas, "relatorio_pessoas.hfd");

    encerrarHash(hashQuadras);
    encerrarHash(hashPessoas);

    free(pathCompletoGeo);
    free(pathCompletoPm);
    free(paths);

    printf("Processamento concluido com sucesso.\n");
    return 0;
}

char* concatenarCaminho(const char* dir, const char* arquivo) {
    char* buffer = malloc(512);
    if (!dir || strlen(dir) == 0)
        strncpy(buffer, arquivo, 511);
    else
        snprintf(buffer, 512, "%s/%s", dir, arquivo);
    return buffer;
}
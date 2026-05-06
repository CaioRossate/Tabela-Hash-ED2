#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashing.h"
#include "geo.h"
#include "qry.h"
#include "pessoa.h"
#include "pm.h"

#define MAX_QUADRAS_REMOVIDAS 4096

typedef enum { ENTRADA, SAIDA, GEO, QUERY, PM } FilePaths;

char* concatenarCaminho(const char* dir, const char* arquivo);

int main(int argc, char* argv[]) {

    char* flags[] = {"-e", "-o", "-f", "-q", "-pm"};
    char** paths = calloc(5, sizeof(char*));

    for (int i = 1; i < argc; i += 2)
        for (int j = 0; j < 5; j++)
            if (i + 1 < argc && strcmp(argv[i], flags[j]) == 0)
                paths[j] = argv[i + 1];

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

    // Geo
    char* pathCompletoGeo = concatenarCaminho(paths[ENTRADA], paths[GEO]);
    processarArquivoGeo(pathCompletoGeo, hashQuadras);

    // Pm
    char* pathCompletoPm = NULL;
    if (paths[PM]) {
        pathCompletoPm = concatenarCaminho(paths[ENTRADA], paths[PM]);
    } else {
        char buf[512];
        strncpy(buf, pathCompletoGeo, sizeof(buf) - 1);
        buf[sizeof(buf)-1] = '\0';
        char* dot = strrchr(buf, '.');
        if (dot) strcpy(dot, ".pm");
        pathCompletoPm = malloc(strlen(buf) + 1);
        strcpy(pathCompletoPm, buf);
    }
    processarArquivoPm(pathCompletoPm, hashPessoas);

    // qry
    if (paths[QUERY]) {
        char* pathCompletoQry = concatenarCaminho(paths[ENTRADA], paths[QUERY]);

        // Monta caminhos de saída
        char geoBase[128], qryBase[128];
        strncpy(geoBase, paths[GEO],   sizeof(geoBase)-1); geoBase[sizeof(geoBase)-1]='\0';
        strncpy(qryBase, paths[QUERY], sizeof(qryBase)-1); qryBase[sizeof(qryBase)-1]='\0';
        char* d = strrchr(geoBase, '.'); if (d) *d = '\0';
        char* b = strrchr(qryBase, '/'); if (b) memmove(qryBase, b+1, strlen(b));

        char pathTxt[512], pathSvg[512];
        snprintf(pathTxt, sizeof(pathTxt), "%s/%s-%s.txt", paths[SAIDA], geoBase, qryBase);
        snprintf(pathSvg, sizeof(pathSvg), "%s/%s-%s.svg", paths[SAIDA], geoBase, qryBase);

        FILE* fTxt = fopen(pathTxt, "w");
        FILE* fSvg = fopen(pathSvg, "w");
        if (!fTxt) printf("Erro ao criar TXT: %s\n", pathTxt);
        if (!fSvg) printf("Erro ao criar SVG: %s\n", pathSvg);

        if (fTxt && fSvg) {
            // Pré-leitura: descobre quais CEPs serão removidos pelo rq
            char ceps_removidos[MAX_QUADRAS_REMOVIDAS][20];
            int n_removidos = coletarCepsRemovidos(pathCompletoQry, ceps_removidos, MAX_QUADRAS_REMOVIDAS);

            // Abre SVG com viewBox calculado
            double vx, vy, vw, vh;
            calcularBBoxCidade(hashQuadras, &vx, &vy, &vw, &vh);
            fprintf(fSvg,
                "<svg xmlns=\"http://www.w3.org/2000/svg\""
                " viewBox=\"%lf %lf %lf %lf\" width=\"100%%\" height=\"100%%\">\n",
                vx, vy, vw, vh);

            // Desenha apenas as quadras que NÃO foram removidas
            gerarCidadeSVG(hashQuadras, fSvg, ceps_removidos, n_removidos);

            // Processa os comandos (atualiza dados e marca SVG/TXT)
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
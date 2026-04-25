#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashing.h"
#include "geo.h"
#include "qry.h"
#include "pessoa.h"

typedef enum { ENTRADA, SAIDA, GEO, QUERY } FilePaths;

char* concatenarCaminho(const char* dir, const char* arquivo);

int main(int argc, char* argv[]) {
    char* flags[] = {"-e", "-o", "-f", "-q"};
    char** paths = calloc(4, sizeof(char*));

    for (int i = 1; i < argc; i += 2) {
        for (int j = 0; j < 4; j++) {
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
        return 1;
    }

    char* pathCompletoGeo = concatenarCaminho(paths[ENTRADA], paths[GEO]);
    processarArquivoGeo(pathCompletoGeo, hashQuadras);

    if (paths[QUERY]) {
        char* pathCompletoQry = concatenarCaminho(paths[ENTRADA], paths[QUERY]);
        
        char pathSaidaTxt[256], pathSaidaSvg[256];
        sprintf(pathSaidaTxt, "%s/resultado.txt", paths[SAIDA]);
        sprintf(pathSaidaSvg, "%s/resultado.svg", paths[SAIDA]);

        FILE* fTxt = fopen(pathSaidaTxt, "w");
        FILE* fSvg = fopen(pathSaidaSvg, "w");

        if (fTxt && fSvg) {
            fprintf(fSvg, "<svg xmlns=\"http://www.w3.org/2000/svg\">\n");
            
            gerarCidadeSVG(hashQuadras, fSvg);
            
            processarArquivoQry(pathCompletoQry, hashQuadras, hashPessoas, fTxt, fSvg);
            
            fprintf(fSvg, "</svg>\n");
            
            fclose(fTxt);
            fclose(fSvg);
        }
        free(pathCompletoQry);
    }

    gerarRelatorioHash(hashQuadras, "relatorio_quadras.hfd");
    gerarRelatorioHash(hashPessoas, "relatorio_pessoas.hfd");

    
    encerrarHash(hashQuadras); 
    encerrarHash(hashPessoas);

    free(pathCompletoGeo);
    free(paths);
    printf("Processamento concluido com sucesso.\n");
    return 0;
}

char* concatenarCaminho(const char* dir, const char* arquivo) {
    char* buffer = malloc(256);
    if (!dir || strlen(dir) == 0) strcpy(buffer, arquivo);
    else sprintf(buffer, "%s/%s", dir, arquivo);
    return buffer;
}
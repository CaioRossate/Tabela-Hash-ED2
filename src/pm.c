#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pm.h"
#include "pessoa.h"

// Processa o arquivo .pm e popula o hash de pessoas
bool processarArquivoPm(const char* path_pm, Hash hash_pessoas) {
    FILE* arquivo = fopen(path_pm, "r");
    if (!arquivo) return false;

    char comando[4];
    while (fscanf(arquivo, "%3s", comando) != EOF) {
        if (strcmp(comando, "p") == 0) {
            char cpf[20], nome[32], sobrenome[32], sexo, nasc[16];
            fscanf(arquivo, "%19s %31s %31s %c %15s", cpf, nome, sobrenome, &sexo, nasc);
            void* pessoa = criarPessoa(cpf, nome, sobrenome, sexo, nasc);
            if (pessoa) {
                inserirHash(hash_pessoas, pessoa);
                destruirPessoa(pessoa);
            }
        } else if (strcmp(comando, "m") == 0) {
            char cpf[20], cep[20], face, compl[32];
            double num;
            fscanf(arquivo, "%19s %19s %c %lf %31s", cpf, cep, &face, &num, compl);
            void* buffer = malloc(getPessoaSize());
            if (buscarHash(hash_pessoas, cpf, buffer)) {
                setEnderecoPessoa(buffer, cep, face, num, compl);
                inserirHash(hash_pessoas, buffer);
            }
            free(buffer);
        }
    }
    fclose(arquivo);
    return true;
}

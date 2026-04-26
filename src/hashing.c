#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "hashing.h"

#define BUCKET_CAPACIDADE 10
#define TAM_CHAVE 128


typedef struct {
    int profundidade_local;
    int quantidade;
    char dados[BUCKET_CAPACIDADE * TAM_CHAVE];
} Bucket;

typedef struct {
    FILE* arquivo;
    int profundidade_global;
    size_t tam_reg;
    int total_indices;
    long* offsets;
} HashExtensivel;

// Funções auxiliares para manipulação de arquivos e hashing

static uint64_t gerar_hash_djb2(const char* chave) {
    uint64_t hash = 5381; //valor inicial aleatorio para ter certeza que não começa com 0
    int c;
    while ((c = *chave++)) hash = ((hash << 5) + hash) + c;
    return hash;
}

static void salvar_bucket(FILE* fp, long offset, Bucket* b) {
    fseek(fp, offset, SEEK_SET);
    fwrite(b, sizeof(Bucket), 1, fp);
}

static void carregar_bucket(FILE* fp, long offset, Bucket* b) {
    fseek(fp, offset, SEEK_SET);
    fread(b, sizeof(Bucket), 1, fp);
}

static long alocar_bucket(HashExtensivel* h, int profundidade) {
    fseek(h->arquivo, 0, SEEK_END);
    long offset = ftell(h->arquivo);
    Bucket b = {0};
    b.profundidade_local = profundidade;
    b.quantidade = 0;
    memset(b.dados, 0, sizeof(b.dados));
    salvar_bucket(h->arquivo, offset, &b);
    return offset;
}

// Parte principal

Hash inicializarHash(const char* nome_arquivo, int profundidade_inicial, size_t tamanho_registro) {
    HashExtensivel* h = (HashExtensivel*)malloc(sizeof(HashExtensivel));
    h->tam_reg = tamanho_registro;
    h->arquivo = fopen(nome_arquivo, "w+b");
    if (!h->arquivo) { free(h); return NULL; }

    h->profundidade_global = profundidade_inicial;
    h->total_indices = 1 << h->profundidade_global;
    h->offsets = (long*)malloc(sizeof(long) * h->total_indices);

    long offset_inicial = alocar_bucket(h, profundidade_inicial);
    for (int i = 0; i < h->total_indices; i++) h->offsets[i] = offset_inicial;

    return (Hash)h;
}

bool inserirHash(Hash h, void* dado) {
    HashExtensivel* hash_ext = (HashExtensivel*)h;
    char* chave = (char*)dado;
    uint64_t v_hash = gerar_hash_djb2(chave);
    int trava_seguranca = 0;

    while (true) {
        // Proteção contra loop infinito em caso de colisões excessivas
        if (trava_seguranca++ > 32) {
            fprintf(stderr, "Erro crítico: Loop de split detectado. Verifique a função de hash.\n");
            return false;
        }

        int idx = v_hash & ((1 << hash_ext->profundidade_global) - 1);
        long offset_alvo = hash_ext->offsets[idx];
        
        Bucket b;
        carregar_bucket(hash_ext->arquivo, offset_alvo, &b);

        // Verificação de existência/atualização
        for (int i = 0; i < b.quantidade; i++) {
            char* chave_b = &b.dados[i * hash_ext->tam_reg];
            if (strcmp(chave_b, chave) == 0) {
                memcpy(chave_b, dado, hash_ext->tam_reg);
                salvar_bucket(hash_ext->arquivo, offset_alvo, &b);
                return true;
            }
        }

        // Inserção simples
        if (b.quantidade < BUCKET_CAPACIDADE) {
            memcpy(&b.dados[b.quantidade * hash_ext->tam_reg], dado, hash_ext->tam_reg);
            b.quantidade++;
            salvar_bucket(hash_ext->arquivo, offset_alvo, &b);
            return true;
        }

        // Split Necessário
        if (b.profundidade_local == hash_ext->profundidade_global) {
            int tam_antigo = hash_ext->total_indices;
            hash_ext->profundidade_global++;
            hash_ext->total_indices = 1 << hash_ext->profundidade_global;
            hash_ext->offsets = realloc(hash_ext->offsets, sizeof(long) * hash_ext->total_indices);
            
            for (int i = 0; i < tam_antigo; i++) {
                hash_ext->offsets[i + tam_antigo] = hash_ext->offsets[i];
            }
        }

        // Criar novo bucket e redistribuir
        int bit_separador = 1 << b.profundidade_local;
        b.profundidade_local++;
        long novo_offset = alocar_bucket(hash_ext, b.profundidade_local);

        // Backup e limpeza do original
        char buffer_backup[BUCKET_CAPACIDADE * TAM_CHAVE];
        int qtd_backup = b.quantidade;
        memcpy(buffer_backup, b.dados, sizeof(b.dados));
        b.quantidade = 0;
        memset(b.dados, 0, sizeof(b.dados));
        salvar_bucket(hash_ext->arquivo, offset_alvo, &b);

        // Atualiza diretório
        for (int i = 0; i < hash_ext->total_indices; i++) {
            if (hash_ext->offsets[i] == offset_alvo && (i & bit_separador)) {
                hash_ext->offsets[i] = novo_offset;
            }
        }

        // Redistribuição dos registros
        for (int i = 0; i < qtd_backup; i++) {
            void* reg_atual = &buffer_backup[i * hash_ext->tam_reg];
            uint64_t h_item = gerar_hash_djb2((char*)reg_atual);
            int idx_item = h_item & ((1 << hash_ext->profundidade_global) - 1);
            long offset_dest = hash_ext->offsets[idx_item];
            
            Bucket b_dest;
            carregar_bucket(hash_ext->arquivo, offset_dest, &b_dest);
            if (b_dest.quantidade < BUCKET_CAPACIDADE) {
                memcpy(&b_dest.dados[b_dest.quantidade * hash_ext->tam_reg], reg_atual, hash_ext->tam_reg);
                b_dest.quantidade++;
                salvar_bucket(hash_ext->arquivo, offset_dest, &b_dest);
            }
        }
    }
}

bool buscarHash(Hash h, char* chave, void* destino) {
    HashExtensivel* hash_ext = (HashExtensivel*)h;
    uint64_t v_hash = gerar_hash_djb2(chave);
    int idx = v_hash & ((1 << hash_ext->profundidade_global) - 1);
    
    Bucket b;
    carregar_bucket(hash_ext->arquivo, hash_ext->offsets[idx], &b);

    for (int i = 0; i < b.quantidade; i++) {
        char* chave_b = &b.dados[i * hash_ext->tam_reg];
        if (strcmp(chave_b, chave) == 0) {
            memcpy(destino, &b.dados[i * hash_ext->tam_reg], hash_ext->tam_reg);
            return true;
        }
    }
    return false;
}

void percorrerHash(Hash h, void* contexto, void (*funcao_visita)(void* registro, void* contexto)) {
    HashExtensivel* hash_ext = (HashExtensivel*)h;
    
    long* visitados = (long*)malloc(sizeof(long) * hash_ext->total_indices);
    int qtd_visitados = 0;

    for (int i = 0; i < hash_ext->total_indices; i++) {
        long offset = hash_ext->offsets[i];
        
        bool ja_foi = false;
        for(int j=0; j<qtd_visitados; j++) {
            if(visitados[j] == offset) {
                ja_foi = true; 
                break;
            }
        }
        if(ja_foi) continue;

        Bucket b;
        carregar_bucket(hash_ext->arquivo, offset, &b);
        for (int j = 0; j < b.quantidade; j++) {
            funcao_visita(&b.dados[j * hash_ext->tam_reg], contexto);
        }
        visitados[qtd_visitados++] = offset;
    }
    free(visitados);
}

bool removerHash(Hash h, char* chave) {
    HashExtensivel* hash_ext = (HashExtensivel*)h;
    uint64_t v_hash = gerar_hash_djb2(chave);
    int idx = v_hash & ((1 << hash_ext->profundidade_global) - 1);
    long offset = hash_ext->offsets[idx];

    Bucket b;
    carregar_bucket(hash_ext->arquivo, offset, &b);

    for (int i = 0; i < b.quantidade; i++) {
        char* chave_b = &b.dados[i * hash_ext->tam_reg];
        if (strcmp(chave_b, chave) == 0) {
            // Substitui o registro a ser removido pelo último registro do bucket para manter semelhança de array
            if (i < b.quantidade - 1) {
                void* ultimo_reg = &b.dados[(b.quantidade - 1) * hash_ext->tam_reg];
                memcpy(chave_b, ultimo_reg, hash_ext->tam_reg);
            }
            b.quantidade--;
            salvar_bucket(hash_ext->arquivo, offset, &b);
            return true;
        }
    }
    return false;
}

int getProfundidadeGlobal(Hash h) {
    return ((HashExtensivel*)h)->profundidade_global;
}

int getQuantidadeBuckets(Hash h) {
    return ((HashExtensivel*)h)->total_indices;
}

void encerrarHash(Hash h) {
    if (!h) return;
    HashExtensivel* hash_ext = (HashExtensivel*)h;
    fclose(hash_ext->arquivo);
    free(hash_ext->offsets);
    free(hash_ext);
}

void gerarRelatorioHash(Hash h, const char* nome_arquivo_relatorio) {
    HashExtensivel* hash_ext = (HashExtensivel*)h;
    FILE* rel = fopen(nome_arquivo_relatorio, "w");
    if (!rel) return;

    fprintf(rel, "--- RELATORIO HASH EXTENSIVEL ---\n");
    fprintf(rel, "Profundidade Global: %d\n", hash_ext->profundidade_global);
    fprintf(rel, "Tamanho do Diretorio: %d\n", hash_ext->total_indices);
    fprintf(rel, "Tamanho do Registro: %zu bytes\n\n", hash_ext->tam_reg);

    // Para evitar imprimir buckets duplicados 
    long* visitados = malloc(sizeof(long) * hash_ext->total_indices);
    int qtd_v = 0;

    for (int i = 0; i < hash_ext->total_indices; i++) {
        long offset = hash_ext->offsets[i];
        bool ja_foi = false;
        for(int j=0; j<qtd_v; j++) if(visitados[j] == offset) { ja_foi = true; break; }
        
        fprintf(rel, "Indice [%d] -> Bucket no Offset (%ld)%s\n", i, offset, ja_foi ? " (ja listado)" : "");
        
        if (!ja_foi) {
            Bucket b;
            carregar_bucket(hash_ext->arquivo, offset, &b);
            fprintf(rel, "   > Profundidade Local: %d\n", b.profundidade_local);
            fprintf(rel, "   > Ocupacao: %d/%d\n", b.quantidade, BUCKET_CAPACIDADE);
            visitados[qtd_v++] = offset;
        }
    }

    free(visitados);
    fclose(rel);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qry.h"
#include "pessoa.h"
#include "geo.h"
#include "hashing.h"

typedef struct {
    int m, f;
    int moradores, sem_tetos;
    FILE* txt;
} ContextoCenso;

typedef struct {
    char cep_removido[20];
    Hash h_p; 
} ContextoRQ;

typedef struct {
    char cep_alvo[20];
    int n, s, l, o;
} ContextoPq;

void contarMoradoresPorFace(void* reg, void* ctx) {
    ContextoPq* c = (ContextoPq*)ctx;
    const char* cep_p = habitante_get_cep(reg);
    
    if (cep_p != NULL && strcmp(cep_p, c->cep_alvo) == 0) {
        char face = habitante_get_face(reg);
        if (face == 'N') c->n++;
        else if (face == 'S') c->s++;
        else if (face == 'L') c->l++;
        else if (face == 'O') c->o++;
    }
}

void visitarCenso(void* reg, void* ctx) {
    ContextoCenso* c = (ContextoCenso*)ctx;
    
    if (habitante_get_sexo(reg) == 'M') {
        c->m++;
    } else {
        c->f++;
    }

    if (ehMorador(reg)) {
        c->moradores++;
    } else {
        c->sem_tetos++;
    }
}

void desvincularMoradores(void* reg, void* ctx) {
    ContextoRQ* c = (ContextoRQ*)ctx;
    
    const char* cep_pessoa = habitante_get_cep(reg);
    if (cep_pessoa != NULL && strcmp(cep_pessoa, c->cep_removido) == 0) {
        desvincularEndereco(reg);
        inserirHash(c->h_p, reg);
    }
}



// Função QRY
void processarArquivoQry(const char* path_qry, Hash h_q, Hash h_p, FILE* fTxt, FILE* fSvg) {
    FILE* arq = fopen(path_qry, "r");
    if (!arq) {
        printf("Erro ao abrir arquivo de consulta: %s\n", path_qry);
        return;
    }

    char comando[10];
    
    
    while (fscanf(arq, "%s", comando) != EOF) {
        
        if (strcmp(comando, "rq") == 0) {
            char cep[20];
            fscanf(arq, "%s", cep);
            comando_rq(h_q, h_p, cep, fTxt, fSvg);
        } 
        else if (strcmp(comando, "Pq") == 0) {
            char cep[20];
            fscanf(arq, "%s", cep);
            comando_Pq(h_q, h_p, cep, fTxt, fSvg);
        }
        else if (strcmp(comando, "censo") == 0) {
            comando_censo(h_p, fTxt);
        }
        else if (strcmp(comando, "h?") == 0) {
            char cpf[20];
            fscanf(arq, "%s", cpf);
            comando_h_pergunta(h_p, cpf, fTxt);
        }
        else if (strcmp(comando, "nasc") == 0) {
            char cpf[20], nome[32], sobrenome[32], sexo, nasc[12];
            fscanf(arq, "%s %s %s %c %s", cpf, nome, sobrenome, &sexo, nasc);
            comando_nasc(h_p, cpf, nome, sobrenome, sexo, nasc);
        }
        else if (strcmp(comando, "rip") == 0) {
            char cpf[20];
            fscanf(arq, "%s", cpf);
            comando_rip(h_p, h_q, cpf, fTxt, fSvg);
        }
        else if (strcmp(comando, "mud") == 0) {
            char cpf[20], cep[20], face, cmpl[30];
            double num;
            fscanf(arq, "%s %s %c %lf %s", cpf, cep, &face, &num, cmpl);
            comando_mud(h_p, cpf, cep, face, num, cmpl, fSvg);
        }
        else if (strcmp(comando, "dspj") == 0) {
            char cpf[20];
            fscanf(arq, "%s", cpf);
            comando_dspj(h_p, cpf, fTxt, fSvg);
        }
        else {
            printf("Comando desconhecido no .qry: %s\n", comando);
        }
    }

    fclose(arq);
}

void comando_rq(Hash h_q, Hash h_p, char* cep, FILE* txt, FILE* svg) {
    (void)svg;
    (void)h_q;
    if (removerHash(h_q, cep)) {
        fprintf(txt, "rq %s: quadra removida. Moradores agora sao sem-tetos.\n", cep);
        
        ContextoRQ ctx;
        strcpy(ctx.cep_removido, cep);
        ctx.h_p = h_p;
        percorrerHash(h_p, &ctx, desvincularMoradores);
    }
}

void comando_Pq(Hash h_q, Hash h_p, char* cep, FILE* txt, FILE* svg) {
    (void)svg;
    (void)h_q;
    ContextoPq ctx = {0};
    strcpy(ctx.cep_alvo, cep);
    
    percorrerHash(h_p, &ctx, contarMoradoresPorFace);

    fprintf(txt, "Pq %s: Norte: %d, Sul: %d, Leste: %d, Oeste: %d\n", cep, ctx.n, ctx.s, ctx.l, ctx.o);
}

void comando_censo(Hash h_p, FILE* txt) {
    ContextoCenso ctx = {0, 0, 0, 0, txt};
    percorrerHash(h_p, &ctx, visitarCenso);
    
    fprintf(txt, "Censo: Homens: %d, Mulheres: %d. Total: %d\n", ctx.m, ctx.f, ctx.m + ctx.f);
    fprintf(txt, "Moradores: %d, Sem-tetos: %d\n", ctx.moradores, ctx.sem_tetos);
}

void comando_h_pergunta(Hash h_p, char* cpf, FILE* txt) {
    void* p_buffer = malloc(getPessoaSize()); 
    if (buscarHash(h_p, cpf, p_buffer)) {
        fprintf(txt, "Resultado h? %s:\n", cpf);
        habitante_print_info(txt, p_buffer);
    } else {
        fprintf(txt, "h? %s: CPF nao encontrado no sistema.\n", cpf);
    }
    free(p_buffer);
}

void comando_nasc(Hash h_p, char* cpf, char* nome, char* sobrenome, char sexo, char* nasc) {
    void* p_buffer = criarPessoa(cpf, nome, sobrenome, sexo, nasc);
    if (p_buffer) {
        inserirHash(h_p, p_buffer); 
        destruirPessoa(p_buffer);
    }
}

void comando_rip(Hash h_p, Hash h_q, char* cpf, FILE* txt, FILE* svg) {
    void* p_buffer = malloc(getPessoaSize());
    void* q_buffer = malloc(getQuadraSize());

    if (buscarHash(h_p, cpf, p_buffer)) {
        const char* cep = habitante_get_cep(p_buffer);
        
        fprintf(txt, "rip %s: %s %s faleceu.\n", cpf, 
                habitante_get_nome(p_buffer), habitante_get_sobrenome(p_buffer));

        // Cruz
        if (cep != NULL && buscarHash(h_q, (char*)cep, q_buffer)) {
            double x = getQuadraX(q_buffer);
            double y = getQuadraY(q_buffer);
            fprintf(svg, "\t<text x=\"%lf\" y=\"%lf\" fill=\"black\" font-size=\"10\">†</text>\n", x, y);
            fprintf(svg, "\t<circle cx=\"%lf\" cy=\"%lf\" r=\"5\" fill=\"none\" stroke=\"red\" />\n", x, y);
        }

        removerHash(h_p, cpf);
    } else {
        fprintf(txt, "rip %s: CPF nao encontrado.\n", cpf);
    }

    free(p_buffer);
    free(q_buffer);
}

void comando_mud(Hash h_p, char* cpf, char* cep, char face, double num, char* cmpl, FILE* svg) {
    (void)svg;
    void* p_buffer = malloc(getPessoaSize());
    if (buscarHash(h_p, cpf, p_buffer)) {
        setEnderecoPessoa(p_buffer, cep, face, (double)num, cmpl);
        inserirHash(h_p, p_buffer);
    }
    free(p_buffer);
}

void comando_dspj(Hash h_p, char* cpf, FILE* txt, FILE* svg) {
    (void)svg;
    void* p_buffer = malloc(getPessoaSize());
    if (buscarHash(h_p, cpf, p_buffer)) {
        fprintf(txt, "dspj %s: %s %s foi despejado.\n", cpf, habitante_get_nome(p_buffer), habitante_get_sobrenome(p_buffer));
        desvincularEndereco(p_buffer);
        inserirHash(h_p, p_buffer); 
    }
    free(p_buffer); 
}


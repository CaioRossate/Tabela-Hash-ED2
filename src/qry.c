#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qry.h"
#include "pessoa.h"
#include "geo.h"
#include "hashing.h"

// Structs de contexto para as funções de visita

typedef struct {
    int m, f;
    int moradores, sem_tetos;
    int moradores_m, moradores_f;
    int sem_tetos_m, sem_tetos_f;
    FILE* txt;
} ContextoCenso;

typedef struct {
    char cep_removido[20];
    Hash h_p;
    FILE* txt;
} ContextoRQ;

typedef struct {
    char cep_alvo[20];
    int n, s, l, o;
} ContextoPq;

// Função de visita

static void contarMoradoresPorFace(void* reg, void* ctx) {
    ContextoPq* c = (ContextoPq*)ctx;
    const char* cep_p = habitante_get_cep(reg);
    if (cep_p && strcmp(cep_p, c->cep_alvo) == 0) {
        char face = habitante_get_face(reg);
        if      (face == 'N') c->n++;
        else if (face == 'S') c->s++;
        else if (face == 'L') c->l++;
        else if (face == 'O') c->o++;
    }
}

static void visitarCenso(void* reg, void* ctx) {
    ContextoCenso* c = (ContextoCenso*)ctx;
    char sexo = habitante_get_sexo(reg);
    int eh_m  = (sexo == 'M');
    if (eh_m) c->m++; else c->f++;
    if (ehMorador(reg)) {
        c->moradores++;
        if (eh_m) c->moradores_m++; else c->moradores_f++;
    } else {
        c->sem_tetos++;
        if (eh_m) c->sem_tetos_m++; else c->sem_tetos_f++;
    }
}

static void desvincularMoradores(void* reg, void* ctx) {
    ContextoRQ* c = (ContextoRQ*)ctx;
    const char* cep_pessoa = habitante_get_cep(reg);
    if (cep_pessoa && strcmp(cep_pessoa, c->cep_removido) == 0) {
        fprintf(c->txt, "   Morador desalojado: CPF %s - %s %s\n",
                habitante_get_cpf(reg),
                habitante_get_nome(reg),
                habitante_get_sobrenome(reg));
        desvincularEndereco(reg);
        inserirHash(c->h_p, reg);
    }
}

// Pré-leitura: coleta CEPs removidos sem executar nada

int coletarCepsRemovidos(const char* path_qry, char ceps_out[][20], int max_ceps) {
    FILE* arq = fopen(path_qry, "r");
    if (!arq) return 0;

    int count = 0;
    char comando[10];
    while (fscanf(arq, "%9s", comando) != EOF) {
        if (strcmp(comando, "rq") == 0) {
            if (count < max_ceps)
                fscanf(arq, "%19s", ceps_out[count++]);
            else {
                char lixo[20];
                fscanf(arq, "%19s", lixo);
            }
        } else {
            // Consome o resto da linha para avançar
            char lixo[512];
            fgets(lixo, sizeof(lixo), arq);
        }
    }
    fclose(arq);
    return count;
}

// Processador principal do .qry

void processarArquivoQry(const char* path_qry, Hash h_q, Hash h_p, FILE* fTxt, FILE* fSvg) {
    FILE* arq = fopen(path_qry, "r");
    if (!arq) {
        printf("Erro ao abrir arquivo de consulta: %s\n", path_qry);
        return;
    }

    char comando[10];
    while (fscanf(arq, "%9s", comando) != EOF) {
        if (strcmp(comando, "rq") == 0) {
            char cep[20];
            fscanf(arq, "%19s", cep);
            comando_rq(h_q, h_p, cep, fTxt, fSvg);
        }
        else if (strcmp(comando, "Pq") == 0 || strcmp(comando, "pq") == 0) {
            char cep[20];
            fscanf(arq, "%19s", cep);
            comando_Pq(h_q, h_p, cep, fTxt, fSvg);
        }
        else if (strcmp(comando, "censo") == 0) {
            comando_censo(h_p, fTxt);
        }
        else if (strcmp(comando, "h?") == 0) {
            char cpf[20];
            fscanf(arq, "%19s", cpf);
            comando_h_pergunta(h_p, cpf, fTxt);
        }
        else if (strcmp(comando, "nasc") == 0) {
            char cpf[20], nome[32], sobrenome[32], nasc[12], sexo_str[4];
            fscanf(arq, "%19s %31s %31s %3s %11s", cpf, nome, sobrenome, sexo_str, nasc);
            comando_nasc(h_p, cpf, nome, sobrenome, sexo_str[0], nasc);
        }
        else if (strcmp(comando, "rip") == 0) {
            char cpf[20];
            fscanf(arq, "%19s", cpf);
            comando_rip(h_p, h_q, cpf, fTxt, fSvg);
        }
        else if (strcmp(comando, "mud") == 0) {
            char cpf[20], cep[20], face_str[4], cmpl[100];
            double num;
            if (fscanf(arq, "%19s %19s %3s %lf %99s", cpf, cep, face_str, &num, cmpl) != 5) continue;
            comando_mud(h_p, h_q, cpf, cep, face_str[0], num, cmpl, fTxt, fSvg);
        }
        else if (strcmp(comando, "dspj") == 0) {
            char cpf[20];
            fscanf(arq, "%19s", cpf);
            comando_dspj(h_p, h_q, cpf, fTxt, fSvg);
        }
        else {
            char lixo[256];
            fgets(lixo, sizeof(lixo), arq);
        }
    }
    fclose(arq);
}

// Comandos

void comando_rq(Hash h_q, Hash h_p, char* cep, FILE* txt, FILE* svg) {
    
    (void)svg;

    fprintf(txt, "rq %s: quadra removida. Moradores agora sao sem-tetos:\n", cep);

    ContextoRQ ctx;
    strncpy(ctx.cep_removido, cep, 19);
    ctx.cep_removido[19] = '\0';
    ctx.h_p  = h_p;
    ctx.txt  = txt;
    percorrerHash(h_p, &ctx, desvincularMoradores);

    removerHash(h_q, cep);
}

void comando_Pq(Hash h_q, Hash h_p, char* cep, FILE* txt, FILE* svg) {
    ContextoPq ctx = {0};
    strncpy(ctx.cep_alvo, cep, 19);
    ctx.cep_alvo[19] = '\0';
    percorrerHash(h_p, &ctx, contarMoradoresPorFace);

    int total = ctx.n + ctx.s + ctx.l + ctx.o;
    fprintf(txt, "Pq %s: Norte: %d, Sul: %d, Leste: %d, Oeste: %d, Total: %d\n",
            cep, ctx.n, ctx.s, ctx.l, ctx.o, total);

    void* q_buffer = malloc(getQuadraSize());
    if (buscarHash(h_q, cep, q_buffer)) {
        double x = getQuadraX(q_buffer), y = getQuadraY(q_buffer);
        double w = getQuadraW(q_buffer), h = getQuadraH(q_buffer);
        fprintf(svg, "\t<text x=\"%lf\" y=\"%lf\" fill=\"blue\" font-size=\"10\" text-anchor=\"middle\">%d</text>\n", x+w/2, y+10, ctx.n);
        fprintf(svg, "\t<text x=\"%lf\" y=\"%lf\" fill=\"blue\" font-size=\"10\" text-anchor=\"middle\">%d</text>\n", x+w/2, y+h-5, ctx.s);
        fprintf(svg, "\t<text x=\"%lf\" y=\"%lf\" fill=\"blue\" font-size=\"10\" text-anchor=\"middle\">%d</text>\n", x+w-5, y+h/2, ctx.l);
        fprintf(svg, "\t<text x=\"%lf\" y=\"%lf\" fill=\"blue\" font-size=\"10\" text-anchor=\"middle\">%d</text>\n", x+15,  y+h/2, ctx.o);
        fprintf(svg, "\t<text x=\"%lf\" y=\"%lf\" fill=\"darkblue\" font-size=\"12\" text-anchor=\"middle\" font-weight=\"bold\">%d</text>\n", x+w/2, y+h/2+4, total);
    }
    free(q_buffer);
}

void comando_censo(Hash h_p, FILE* txt) {
    ContextoCenso ctx = {0};
    percorrerHash(h_p, &ctx, visitarCenso);
    int total = ctx.m + ctx.f;
    fprintf(txt, "=== CENSO DE BITNOPOLIS ===\n");
    fprintf(txt, "Total de habitantes : %d\n", total);
    fprintf(txt, "Total de moradores  : %d\n", ctx.moradores);
    fprintf(txt, "Total de sem-tetos  : %d\n", ctx.sem_tetos);
    if (total > 0) {
        fprintf(txt, "Proporcao moradores/habitantes: %.1f%%\n", 100.0*ctx.moradores/total);
        fprintf(txt, "Homens : %d (%.1f%% dos habitantes)\n", ctx.m, 100.0*ctx.m/total);
        fprintf(txt, "Mulheres: %d (%.1f%% dos habitantes)\n", ctx.f, 100.0*ctx.f/total);
    }
    if (ctx.moradores > 0) {
        fprintf(txt, "Moradores homens  : %d (%.1f%% dos moradores)\n", ctx.moradores_m, 100.0*ctx.moradores_m/ctx.moradores);
        fprintf(txt, "Moradores mulheres: %d (%.1f%% dos moradores)\n", ctx.moradores_f, 100.0*ctx.moradores_f/ctx.moradores);
    }
    if (ctx.sem_tetos > 0) {
        fprintf(txt, "Sem-tetos homens  : %d (%.1f%% dos sem-tetos)\n", ctx.sem_tetos_m, 100.0*ctx.sem_tetos_m/ctx.sem_tetos);
        fprintf(txt, "Sem-tetos mulheres: %d (%.1f%% dos sem-tetos)\n", ctx.sem_tetos_f, 100.0*ctx.sem_tetos_f/ctx.sem_tetos);
    }
}

void comando_h_pergunta(Hash h_p, char* cpf, FILE* txt) {
    void* p = malloc(getPessoaSize());
    if (buscarHash(h_p, cpf, p)) {
        fprintf(txt, "Resultado h? %s:\n", cpf);
        habitante_print_info(txt, p);
    } else {
        fprintf(txt, "h? %s: CPF nao encontrado no sistema.\n", cpf);
    }
    free(p);
}

void comando_nasc(Hash h_p, char* cpf, char* nome, char* sobrenome, char sexo, char* nasc) {
    void* p = criarPessoa(cpf, nome, sobrenome, sexo, nasc);
    if (p) { inserirHash(h_p, p); destruirPessoa(p); }
}

void comando_rip(Hash h_p, Hash h_q, char* cpf, FILE* txt, FILE* svg) {
    void* p = malloc(getPessoaSize());
    void* q = malloc(getQuadraSize());
    if (buscarHash(h_p, cpf, p)) {
        const char* cep = habitante_get_cep(p);
        fprintf(txt, "rip %s: %s %s faleceu.\n", cpf, habitante_get_nome(p), habitante_get_sobrenome(p));
        if (cep) {
            fprintf(txt, "   Endereco: CEP %s, Face %c, Num %.0lf\n", cep, habitante_get_face(p), habitante_get_numero(p));
            if (buscarHash(h_q, (char*)cep, q)) {
                double x = getQuadraX(q), y = getQuadraY(q);
                double w = getQuadraW(q), hh = getQuadraH(q);
                char face = habitante_get_face(p);
                double num = habitante_get_numero(p);
                double ex = x, ey = y;
                if      (face=='N') { ex=x+(num/100.0)*w; ey=y; }
                else if (face=='S') { ex=x+(num/100.0)*w; ey=y+hh; }
                else if (face=='L') { ex=x+w; ey=y+(num/100.0)*hh; }
                else if (face=='O') { ex=x;   ey=y+(num/100.0)*hh; }
                fprintf(svg, "\t<line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\" stroke=\"red\" stroke-width=\"2\"/>\n", ex-5,ey-5,ex+5,ey+5);
                fprintf(svg, "\t<line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\" stroke=\"red\" stroke-width=\"2\"/>\n", ex-5,ey+5,ex+5,ey-5);
            }
        }
        removerHash(h_p, cpf);
    } else {
        fprintf(txt, "rip %s: CPF nao encontrado.\n", cpf);
    }
    free(p); free(q);
}

void comando_mud(Hash h_p, Hash h_q, char* cpf, char* cep, char face, double num, char* cmpl, FILE* txt, FILE* svg) {
    void* p = malloc(getPessoaSize());
    void* q = malloc(getQuadraSize());
    if (buscarHash(h_p, cpf, p)) {
        fprintf(txt, "mud %s: %s %s mudou para CEP %s, Face %c, Num %.0lf, Cmpl %s\n",
                cpf, habitante_get_nome(p), habitante_get_sobrenome(p), cep, face, num, cmpl);
        setEnderecoPessoa(p, cep, face, num, cmpl);
        inserirHash(h_p, p);
        if (buscarHash(h_q, cep, q)) {
            double x=getQuadraX(q), y=getQuadraY(q), w=getQuadraW(q), hh=getQuadraH(q);
            double ex=x, ey=y;
            if      (face=='N') { ex=x+(num/100.0)*w; ey=y; }
            else if (face=='S') { ex=x+(num/100.0)*w; ey=y+hh; }
            else if (face=='L') { ex=x+w; ey=y+(num/100.0)*hh; }
            else if (face=='O') { ex=x;   ey=y+(num/100.0)*hh; }
            fprintf(svg, "\t<rect x=\"%lf\" y=\"%lf\" width=\"20\" height=\"15\" fill=\"red\" stroke=\"darkred\" stroke-width=\"1\"/>\n", ex-10, ey-7.5);
            fprintf(svg, "\t<text x=\"%lf\" y=\"%lf\" fill=\"white\" font-size=\"6\" text-anchor=\"middle\">%s</text>\n", ex, ey+2, cpf);
        }
    } else {
        fprintf(txt, "mud %s: CPF nao encontrado.\n", cpf);
    }
    free(p); free(q);
}

void comando_dspj(Hash h_p, Hash h_q, char* cpf, FILE* txt, FILE* svg) {
    void* p = malloc(getPessoaSize());
    void* q = malloc(getQuadraSize());
    if (buscarHash(h_p, cpf, p)) {
        const char* cep = habitante_get_cep(p);
        char face = habitante_get_face(p);
        double num = habitante_get_numero(p);
        fprintf(txt, "dspj %s: %s %s foi despejado.\n", cpf, habitante_get_nome(p), habitante_get_sobrenome(p));
        if (cep) {
            fprintf(txt, "   Endereco do despejo: CEP %s, Face %c, Num %.0lf\n", cep, face, num);
            if (buscarHash(h_q, (char*)cep, q)) {
                double x=getQuadraX(q), y=getQuadraY(q), w=getQuadraW(q), hh=getQuadraH(q);
                double ex=x, ey=y;
                if      (face=='N') { ex=x+(num/100.0)*w; ey=y; }
                else if (face=='S') { ex=x+(num/100.0)*w; ey=y+hh; }
                else if (face=='L') { ex=x+w; ey=y+(num/100.0)*hh; }
                else if (face=='O') { ex=x;   ey=y+(num/100.0)*hh; }
                fprintf(svg, "\t<circle cx=\"%lf\" cy=\"%lf\" r=\"8\" fill=\"black\" stroke=\"black\" stroke-width=\"1\"/>\n", ex, ey);
            }
        } else {
            fprintf(txt, "   Situacao: sem-teto (sem endereco para despejar).\n");
        }
        desvincularEndereco(p);
        inserirHash(h_p, p);
    } else {
        fprintf(txt, "dspj %s: CPF nao encontrado.\n", cpf);
    }
    free(p); free(q);
}
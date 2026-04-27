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


void contarMoradoresPorFace(void* reg, void* ctx) {
    ContextoPq* c = (ContextoPq*)ctx;
    const char* cep_p = habitante_get_cep(reg);

    if (cep_p != NULL && strcmp(cep_p, c->cep_alvo) == 0) {
        char face = habitante_get_face(reg);
        if      (face == 'N') c->n++;
        else if (face == 'S') c->s++;
        else if (face == 'L') c->l++;
        else if (face == 'O') c->o++;
    }
}

void visitarCenso(void* reg, void* ctx) {
    ContextoCenso* c = (ContextoCenso*)ctx;
    char sexo = habitante_get_sexo(reg);
    int eh_m = (sexo == 'M');

    if (eh_m) c->m++; else c->f++;

    if (ehMorador(reg)) {
        c->moradores++;
        if (eh_m) c->moradores_m++; else c->moradores_f++;
    } else {
        c->sem_tetos++;
        if (eh_m) c->sem_tetos_m++; else c->sem_tetos_f++;
    }
}

void desvincularMoradores(void* reg, void* ctx) {
    ContextoRQ* c = (ContextoRQ*)ctx;
    const char* cep_pessoa = habitante_get_cep(reg);

    if (cep_pessoa != NULL && strcmp(cep_pessoa, c->cep_removido) == 0) {
        fprintf(c->txt, "   Morador desalojado: CPF %s - %s %s\n",
                habitante_get_cpf(reg),
                habitante_get_nome(reg),
                habitante_get_sobrenome(reg));

        desvincularEndereco(reg);
        inserirHash(c->h_p, reg);
    }
}

// Qry

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
            char sexo;
            fscanf(arq, "%19s %31s %31s %3s %11s", cpf, nome, sobrenome, sexo_str, nasc);
            sexo = sexo_str[0];
            comando_nasc(h_p, cpf, nome, sobrenome, sexo, nasc);
        }
        else if (strcmp(comando, "rip") == 0) {
            char cpf[20];
            fscanf(arq, "%19s", cpf);
            comando_rip(h_p, h_q, cpf, fTxt, fSvg);
        }
        else if (strcmp(comando, "mud") == 0) {
            char cpf[20], cep[20], face_str[4], cmpl[100];
            double num;
            char face;
            if (fscanf(arq, "%19s %19s %3s %lf %99s", cpf, cep, face_str, &num, cmpl) != 5) continue;
            face = face_str[0];
            comando_mud(h_p, h_q, cpf, cep, face, num, cmpl, fTxt, fSvg);
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
    void* q_buffer = malloc(getQuadraSize());

    if (buscarHash(h_q, cep, q_buffer)) {
        double x = getQuadraX(q_buffer);
        double y = getQuadraY(q_buffer);

        // X vermelho na âncora da quadra
        fprintf(svg, "\t<line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\" stroke=\"red\" stroke-width=\"2\"/>\n",
                x - 5, y - 5, x + 5, y + 5);
        fprintf(svg, "\t<line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\" stroke=\"red\" stroke-width=\"2\"/>\n",
                x - 5, y + 5, x + 5, y - 5);
    }

    fprintf(txt, "rq %s: quadra removida. Moradores agora sao sem-tetos:\n", cep);

    // Lista cada morador desalojado no TXT e depois desvincula
    ContextoRQ ctx;
    strncpy(ctx.cep_removido, cep, 19);
    ctx.cep_removido[19] = '\0';
    ctx.h_p = h_p;
    ctx.txt = txt;
    percorrerHash(h_p, &ctx, desvincularMoradores);

    removerHash(h_q, cep);

    free(q_buffer);
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
        double x = getQuadraX(q_buffer);
        double y = getQuadraY(q_buffer);
        double w = getQuadraW(q_buffer);
        double h = getQuadraH(q_buffer);

        // Norte
        fprintf(svg, "\t<text x=\"%lf\" y=\"%lf\" fill=\"blue\" font-size=\"10\" text-anchor=\"middle\">%d</text>\n",
                x + w / 2, y + 10, ctx.n);
        // Sul
        fprintf(svg, "\t<text x=\"%lf\" y=\"%lf\" fill=\"blue\" font-size=\"10\" text-anchor=\"middle\">%d</text>\n",
                x + w / 2, y + h - 5, ctx.s);
        // Leste
        fprintf(svg, "\t<text x=\"%lf\" y=\"%lf\" fill=\"blue\" font-size=\"10\" text-anchor=\"middle\">%d</text>\n",
                x + w - 5, y + h / 2, ctx.l);
        // Oeste
        fprintf(svg, "\t<text x=\"%lf\" y=\"%lf\" fill=\"blue\" font-size=\"10\" text-anchor=\"middle\">%d</text>\n",
                x + 15, y + h / 2, ctx.o);
        // Total no centro
        fprintf(svg, "\t<text x=\"%lf\" y=\"%lf\" fill=\"darkblue\" font-size=\"12\" text-anchor=\"middle\" font-weight=\"bold\">%d</text>\n",
                x + w / 2, y + h / 2 + 4, total);
    }
    free(q_buffer);
}

void comando_censo(Hash h_p, FILE* txt) {
    ContextoCenso ctx = {0};
    ctx.txt = txt;
    percorrerHash(h_p, &ctx, visitarCenso);

    int total = ctx.m + ctx.f;

    fprintf(txt, "=== CENSO DE BITNOPOLIS ===\n");
    fprintf(txt, "Total de habitantes : %d\n", total);
    fprintf(txt, "Total de moradores  : %d\n", ctx.moradores);
    fprintf(txt, "Total de sem-tetos  : %d\n", ctx.sem_tetos);

    if (total > 0) {
        fprintf(txt, "Proporcao moradores/habitantes: %.1f%%\n",
                100.0 * ctx.moradores / total);
        fprintf(txt, "Homens : %d (%.1f%% dos habitantes)\n",
                ctx.m, 100.0 * ctx.m / total);
        fprintf(txt, "Mulheres: %d (%.1f%% dos habitantes)\n",
                ctx.f, 100.0 * ctx.f / total);
    }

    if (ctx.moradores > 0) {
        fprintf(txt, "Moradores homens  : %d (%.1f%% dos moradores)\n",
                ctx.moradores_m, 100.0 * ctx.moradores_m / ctx.moradores);
        fprintf(txt, "Moradores mulheres: %d (%.1f%% dos moradores)\n",
                ctx.moradores_f, 100.0 * ctx.moradores_f / ctx.moradores);
    }

    if (ctx.sem_tetos > 0) {
        fprintf(txt, "Sem-tetos homens  : %d (%.1f%% dos sem-tetos)\n",
                ctx.sem_tetos_m, 100.0 * ctx.sem_tetos_m / ctx.sem_tetos);
        fprintf(txt, "Sem-tetos mulheres: %d (%.1f%% dos sem-tetos)\n",
                ctx.sem_tetos_f, 100.0 * ctx.sem_tetos_f / ctx.sem_tetos);
    }
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

        if (cep != NULL) {
            fprintf(txt, "   Endereço: CEP %s, Face %c, Num %.0lf\n",
                    cep, habitante_get_face(p_buffer), habitante_get_numero(p_buffer));

            if (buscarHash(h_q, (char*)cep, q_buffer)) {
                double x = getQuadraX(q_buffer);
                double y = getQuadraY(q_buffer);
                double w = getQuadraW(q_buffer);
                double h = getQuadraH(q_buffer);
                char face = habitante_get_face(p_buffer);
                double num = habitante_get_numero(p_buffer);

                double end_x = x, end_y = y;
                if      (face == 'N') { end_x = x + (num / 100.0) * w; end_y = y; }
                else if (face == 'S') { end_x = x + (num / 100.0) * w; end_y = y + h; }
                else if (face == 'L') { end_x = x + w; end_y = y + (num / 100.0) * h; }
                else if (face == 'O') { end_x = x;     end_y = y + (num / 100.0) * h; }

                // Cruz vermelha no local do endereço
                fprintf(svg, "\t<line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\" stroke=\"red\" stroke-width=\"2\"/>\n",
                        end_x - 5, end_y - 5, end_x + 5, end_y + 5);
                fprintf(svg, "\t<line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\" stroke=\"red\" stroke-width=\"2\"/>\n",
                        end_x - 5, end_y + 5, end_x + 5, end_y - 5);
            }
        }

        removerHash(h_p, cpf);
    } else {
        fprintf(txt, "rip %s: CPF nao encontrado.\n", cpf);
    }

    free(p_buffer);
    free(q_buffer);
}

void comando_mud(Hash h_p, Hash h_q, char* cpf, char* cep, char face, double num, char* cmpl, FILE* txt, FILE* svg) {
    void* p_buffer = malloc(getPessoaSize());
    void* q_buffer = malloc(getQuadraSize());

    if (buscarHash(h_p, cpf, p_buffer)) {
        fprintf(txt, "mud %s: %s %s mudou para CEP %s, Face %c, Num %.0lf, Cmpl %s\n",
                cpf, habitante_get_nome(p_buffer), habitante_get_sobrenome(p_buffer),
                cep, face, num, cmpl);

        setEnderecoPessoa(p_buffer, cep, face, num, cmpl);
        inserirHash(h_p, p_buffer);

        if (buscarHash(h_q, cep, q_buffer)) {
            double x = getQuadraX(q_buffer);
            double y = getQuadraY(q_buffer);
            double w = getQuadraW(q_buffer);
            double h = getQuadraH(q_buffer);

            double end_x = x, end_y = y;
            if      (face == 'N') { end_x = x + (num / 100.0) * w; end_y = y; }
            else if (face == 'S') { end_x = x + (num / 100.0) * w; end_y = y + h; }
            else if (face == 'L') { end_x = x + w; end_y = y + (num / 100.0) * h; }
            else if (face == 'O') { end_x = x;     end_y = y + (num / 100.0) * h; }

            // Quadrado vermelho com CPF
            fprintf(svg, "\t<rect x=\"%lf\" y=\"%lf\" width=\"20\" height=\"15\" fill=\"red\" stroke=\"darkred\" stroke-width=\"1\"/>\n",
                    end_x - 10, end_y - 7.5);
            fprintf(svg, "\t<text x=\"%lf\" y=\"%lf\" fill=\"white\" font-size=\"6\" text-anchor=\"middle\">%s</text>\n",
                    end_x, end_y + 2, cpf);
        }
    } else {
        fprintf(txt, "mud %s: CPF nao encontrado.\n", cpf);
    }

    free(p_buffer);
    free(q_buffer);
}

void comando_dspj(Hash h_p, Hash h_q, char* cpf, FILE* txt, FILE* svg) {
    void* p_buffer = malloc(getPessoaSize());
    void* q_buffer = malloc(getQuadraSize());

    if (buscarHash(h_p, cpf, p_buffer)) {
        const char* cep = habitante_get_cep(p_buffer);
        char face = habitante_get_face(p_buffer);
        double num = habitante_get_numero(p_buffer);

        fprintf(txt, "dspj %s: %s %s foi despejado.\n", cpf,
                habitante_get_nome(p_buffer), habitante_get_sobrenome(p_buffer));

        if (cep != NULL) {
            fprintf(txt, "   Endereço do despejo: CEP %s, Face %c, Num %.0lf\n",
                    cep, face, num);

            if (buscarHash(h_q, (char*)cep, q_buffer)) {
                double x = getQuadraX(q_buffer);
                double y = getQuadraY(q_buffer);
                double w = getQuadraW(q_buffer);
                double h = getQuadraH(q_buffer);

                double end_x = x, end_y = y;
                if      (face == 'N') { end_x = x + (num / 100.0) * w; end_y = y; }
                else if (face == 'S') { end_x = x + (num / 100.0) * w; end_y = y + h; }
                else if (face == 'L') { end_x = x + w; end_y = y + (num / 100.0) * h; }
                else if (face == 'O') { end_x = x;     end_y = y + (num / 100.0) * h; }

                // Círculo preto no local do despejo
                fprintf(svg, "\t<circle cx=\"%lf\" cy=\"%lf\" r=\"8\" fill=\"black\" stroke=\"black\" stroke-width=\"1\"/>\n",
                        end_x, end_y);
            }
        } else {
            fprintf(txt, "   Situacao: sem-teto (sem endereço para despejar).\n");
        }

        desvincularEndereco(p_buffer);
        inserirHash(h_p, p_buffer);
    } else {
        fprintf(txt, "dspj %s: CPF nao encontrado.\n", cpf);
    }

    free(p_buffer);
    free(q_buffer);
}
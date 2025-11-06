#include "bd_times.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

// Remove caracteres de nova linha deixados por fgets.
static void chomp(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[--n] = '\0';
    }
}

void bdtimes_init(BDTimes *bd) {
    // Comeca com zero registros carregados
    bd->n = 0;
}

void time_zerar_stats(Time *t) {
    // Limpa estatisticas acumuladas (usado antes de aplicar partidas)
    t->v = 0;
    t->e = 0;
    t->d = 0;
    t->gm = 0;
    t->gs = 0;
}

void time_acumular_partida(Time *t, int gols_feitos, int gols_sofridos) {
    // Atualiza gols e resultado (vitoria/empate/derrota) para um unico jogo
    t->gm += gols_feitos;
    t->gs += gols_sofridos;
    if (gols_feitos > gols_sofridos) {
        t->v++;
    } else if (gols_feitos == gols_sofridos) {
        t->e++;
    } else {
        t->d++;
    }
}

int time_pontos(const Time *t) {
    return t->v * 3 + t->e;
}

int time_saldo(const Time *t) {
    return t->gm - t->gs;
}

// Quebra linha CSV (ID,Nome) em campos prontos para popular Time.
static int parse_time_linha(char *linha, int *id, char *nome) {
    chomp(linha);
    str_trim(linha);

    char *tok = strtok(linha, ",");
    if (!tok) return 0;
    int tmp_id;
    if (!safe_atoi(tok, &tmp_id)) return 0;

    tok = strtok(NULL, ",");
    if (!tok) return 0;
    str_trim(tok);
    chomp(tok);

    *id = tmp_id;
    strncpy(nome, tok, MAX_NOME_TIME - 1);
    nome[MAX_NOME_TIME - 1] = '\0';
    return 1;
}

int bdtimes_carregar_csv(BDTimes *bd, const char *caminho) {
    FILE *f = fopen(caminho, "r");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo de times: %s\n", caminho);
        return 0;
    }
    char buf[256];

    // Descarte do cabecalho
    if (!fgets(buf, sizeof(buf), f)) {
        fclose(f);
        fprintf(stderr, "Arquivo de times vazio ou invalido: %s\n", caminho);
        return 0;
    }

    int count = 0;
    while (fgets(buf, sizeof(buf), f)) {
        if (bd->n >= MAX_TIMES) {
            fprintf(stderr, "Limite de times atingido (%d)\n", MAX_TIMES);
            break;
        }
        char linha[256];
        strncpy(linha, buf, sizeof(linha) - 1);
        linha[sizeof(linha) - 1] = '\0';

        int id;
        char nome[MAX_NOME_TIME];
        if (!parse_time_linha(linha, &id, nome)) {
            fprintf(stderr, "Linha de time ignorada (parse falhou): %s", buf);
            continue;
        }
        Time t;
        t.id = id;
        strncpy(t.nome, nome, MAX_NOME_TIME);
        t.nome[MAX_NOME_TIME - 1] = '\0';
        time_zerar_stats(&t);
        bd->times[bd->n++] = t;
        count++;
    }
    fclose(f);
    return count;
}

Time* bdtimes_buscar_por_id(BDTimes *bd, int id) {
    for (int i = 0; i < bd->n; i++) {
        if (bd->times[i].id == id) return &bd->times[i];
    }
    return NULL;
}

int bdtimes_buscar_por_prefixo(BDTimes *bd, const char *prefixo, int *indices, int max_indices) {
    int found = 0;
    for (int i = 0; i < bd->n; i++) {
        if (str_starts_with_case_insensitive(bd->times[i].nome, prefixo)) {
            if (found < max_indices) indices[found] = i;
            found++;
        }
    }
    return found;
}

void bdtimes_imprimir_classificacao(const BDTimes *bd) {
    // Larguras visuais de cada coluna na tabela
    const int W_ID   = 3;
    const int W_TIME = 12;
    const int W_V    = 2, W_E = 2, W_D = 2, W_GM = 3, W_GS = 3, W_S = 3, W_PG = 3;

    // Cabecalho
    printf("| "); print_utf8_padded("ID", W_ID);
    printf(" | "); print_utf8_padded("Time", W_TIME);
    printf(" | "); print_utf8_padded("V", W_V);
    printf(" | "); print_utf8_padded("E", W_E);
    printf(" | "); print_utf8_padded("D", W_D);
    printf(" | "); print_utf8_padded("GM", W_GM);
    printf(" | "); print_utf8_padded("GS", W_GS);
    printf(" | "); print_utf8_padded("S", W_S);
    printf(" | "); print_utf8_padded("PG", W_PG);
    printf(" |\n");

    // Linha separadora para alinhar a tabela
    printf("|-"); for (int i = 0; i < W_ID; i++) putchar('-');
    printf("-|-"); for (int i = 0; i < W_TIME; i++) putchar('-');
    printf("-|-"); for (int i = 0; i < W_V; i++) putchar('-');
    printf("-|-"); for (int i = 0; i < W_E; i++) putchar('-');
    printf("-|-"); for (int i = 0; i < W_D; i++) putchar('-');
    printf("-|-"); for (int i = 0; i < W_GM; i++) putchar('-');
    printf("-|-"); for (int i = 0; i < W_GS; i++) putchar('-');
    printf("-|-"); for (int i = 0; i < W_S; i++) putchar('-');
    printf("-|-"); for (int i = 0; i < W_PG; i++) putchar('-');
    printf("-|\n");

    // Imprime pela ordem de ID (0..9999) para manter saida previsivel
    for (int id = 0; id < 10000; id++) {
        for (int i = 0; i < bd->n; i++) {
            if (bd->times[i].id == id) {
                const Time *t = &bd->times[i];
                char tmp[32];

                printf("| ");
                snprintf(tmp, sizeof(tmp), "%d", t->id);
                print_utf8_padded(tmp, W_ID);

                printf(" | ");
                print_utf8_padded(t->nome, W_TIME);

                printf(" | ");
                snprintf(tmp, sizeof(tmp), "%d", t->v);
                print_utf8_padded(tmp, W_V);

                printf(" | ");
                snprintf(tmp, sizeof(tmp), "%d", t->e);
                print_utf8_padded(tmp, W_E);

                printf(" | ");
                snprintf(tmp, sizeof(tmp), "%d", t->d);
                print_utf8_padded(tmp, W_D);

                printf(" | ");
                snprintf(tmp, sizeof(tmp), "%d", t->gm);
                print_utf8_padded(tmp, W_GM);

                printf(" | ");
                snprintf(tmp, sizeof(tmp), "%d", t->gs);
                print_utf8_padded(tmp, W_GS);

                printf(" | ");
                snprintf(tmp, sizeof(tmp), "%d", time_saldo(t));
                print_utf8_padded(tmp, W_S);

                printf(" | ");
                snprintf(tmp, sizeof(tmp), "%d", time_pontos(t));
                print_utf8_padded(tmp, W_PG);

                printf(" |\n");
            }
        }
    }
}

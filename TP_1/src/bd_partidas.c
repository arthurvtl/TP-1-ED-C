#include "bd_partidas.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void bdpartidas_init(BDPartidas *bd) {
    // Zera o contador, deixando a base pronta para receber partidas
    bd->n = 0;
}

// Remove caracteres de fim de linha de forma segura.
static void chomp(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[--n] = '\0';
    }
}

// Converte uma linha CSV (ID,Time1ID,Time2ID,Gols1,Gols2) em uma estrutura Partida.
static int parse_partida_linha(char *linha, Partida *out) {
    chomp(linha);
    str_trim(linha);

    char *tok;
    int vals[5];
    int i = 0;

    tok = strtok(linha, ",");
    while (tok && i < 5) {
        str_trim(tok);
        chomp(tok);
        int v;
        if (!safe_atoi(tok, &v)) {
            return 0;
        }
        vals[i++] = v;
        tok = strtok(NULL, ",");
    }
    if (i != 5) return 0;

    out->id    = vals[0];
    out->time1 = vals[1];
    out->time2 = vals[2];
    out->g1    = vals[3];
    out->g2    = vals[4];
    return 1;
}

int bdpartidas_carregar_csv(BDPartidas *bd, const char *caminho) {
    FILE *f = fopen(caminho, "r");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo de partidas: %s\n", caminho);
        return 0;
    }
    char buf[512];

    // Descarta linha de cabeçalho
    if (!fgets(buf, sizeof(buf), f)) {
        fclose(f);
        fprintf(stderr, "Arquivo de partidas vazio ou invalido: %s\n", caminho);
        return 0;
    }

    int count = 0;
    while (fgets(buf, sizeof(buf), f)) {
        if (bd->n >= MAX_PARTIDAS) {
            fprintf(stderr, "Limite de partidas atingido (%d)\n", MAX_PARTIDAS);
            break;
        }
        char linha[512];
        strncpy(linha, buf, sizeof(linha) - 1);
        linha[sizeof(linha) - 1] = '\0';

        Partida p;
        if (!parse_partida_linha(linha, &p)) {
            fprintf(stderr, "Linha de partida ignorada (parse falhou): %s", buf);
            continue;
        }
        bd->partidas[bd->n++] = p;
        count++;
    }
    fclose(f);
    return count;
}

// Aplica os resultados das partidas nas estatisticas acumuladas de cada time.
void bdpartidas_aplicar_em_bdtimes(const BDPartidas *bdp, BDTimes *bdt) {
    for (int i = 0; i < bdp->n; i++) {
        const Partida *p = &bdp->partidas[i];
        Time *t1 = bdtimes_buscar_por_id(bdt, p->time1);
        Time *t2 = bdtimes_buscar_por_id(bdt, p->time2);
        if (!t1 || !t2) {
            fprintf(stderr, "Aviso: partida %d referencia time inexistente (%d,%d)\n", p->id, p->time1, p->time2);
            continue;
        }
        time_acumular_partida(t1, p->g1, p->g2);
        time_acumular_partida(t2, p->g2, p->g1);
    }
}

static const char* nome_do_time(const BDTimes *bdt, int id) {
    for (int i = 0; i < bdt->n; i++) {
        if (bdt->times[i].id == id) return bdt->times[i].nome;
    }
    return "(desconhecido)";
}

// Lista partidas filtrando pelo prefixo do time mandante.
void bdpartidas_listar_por_mandante_prefixo(const BDPartidas *bdp, const BDTimes *bdt, const char *prefixo) {
    int count = 0;
    printf("| ID | Time1 |  | Time2 |\n");
    printf("|----|-------|--|-------|\n");
    for (int i = 0; i < bdp->n; i++) {
        const Partida *p = &bdp->partidas[i];
        const char *n1 = nome_do_time(bdt, p->time1);
        if (str_starts_with_case_insensitive(n1, prefixo)) {
            const char *n2 = nome_do_time(bdt, p->time2);
            printf("| %d | %s | %d x %d | %s |\n", p->id, n1, p->g1, p->g2, n2);
            count++;
        }
    }
    if (count == 0) {
        printf("Nenhuma partida encontrada para mandante com prefixo: %s\n", prefixo);
    }
}

// Lista partidas filtrando pelo prefixo do time visitante.
void bdpartidas_listar_por_visitante_prefixo(const BDPartidas *bdp, const BDTimes *bdt, const char *prefixo) {
    int count = 0;
    printf("| ID | Time1 |  | Time2 |\n");
    printf("|----|-------|--|-------|\n");
    for (int i = 0; i < bdp->n; i++) {
        const Partida *p = &bdp->partidas[i];
        const char *n2 = nome_do_time(bdt, p->time2);
        if (str_starts_with_case_insensitive(n2, prefixo)) {
            const char *n1 = nome_do_time(bdt, p->time1);
            printf("| %d | %s | %d x %d | %s |\n", p->id, n1, p->g1, p->g2, n2);
            count++;
        }
    }
    if (count == 0) {
        printf("Nenhuma partida encontrada para visitante com prefixo: %s\n", prefixo);
    }
}

// Lista partidas quando qualquer um dos times combina com o prefixo.
void bdpartidas_listar_por_qualquer_prefixo(const BDPartidas *bdp, const BDTimes *bdt, const char *prefixo) {
    int count = 0;
    printf("| ID | Time1 |  | Time2 |\n");
    printf("|----|-------|--|-------|\n");
    for (int i = 0; i < bdp->n; i++) {
        const Partida *p = &bdp->partidas[i];
        const char *n1 = nome_do_time(bdt, p->time1);
        const char *n2 = nome_do_time(bdt, p->time2);
        if (str_starts_with_case_insensitive(n1, prefixo) || str_starts_with_case_insensitive(n2, prefixo)) {
            printf("| %d | %s | %d x %d | %s |\n", p->id, n1, p->g1, p->g2, n2);
            count++;
        }
    }
    if (count == 0) {
        printf("Nenhuma partida encontrada para mandante ou visitante com prefixo: %s\n", prefixo);
    }
}

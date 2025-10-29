#ifndef BD_TIMES_H
#define BD_TIMES_H

#include <stddef.h>

#define MAX_TIMES 64
#define MAX_NOME_TIME 64

typedef struct {
    int id;
    char nome[MAX_NOME_TIME];
    int v;
    int e;
    int d;
    int gm;
    int gs;
} Time;

typedef struct {
    Time times[MAX_TIMES];
    int n;
} BDTimes;

void bdtimes_init(BDTimes *bd);
int bdtimes_carregar_csv(BDTimes *bd, const char *caminho);
Time* bdtimes_buscar_por_id(BDTimes *bd, int id);
int bdtimes_buscar_por_prefixo(BDTimes *bd, const char *prefixo, int *indices, int max_indices);

void time_zerar_stats(Time *t);
void time_acumular_partida(Time *t, int gols_feitos, int gols_sofridos);
int time_pontos(const Time *t);
int time_saldo(const Time *t);

void bdtimes_imprimir_classificacao(const BDTimes *bd);

#endif
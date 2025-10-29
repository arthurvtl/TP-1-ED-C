#ifndef BD_PARTIDAS_H
#define BD_PARTIDAS_H

#include <stddef.h>
#include "bd_times.h"

#define MAX_PARTIDAS 500

typedef struct {
    int id;
    int time1;
    int time2;
    int g1;
    int g2;
} Partida;

typedef struct {
    Partida partidas[MAX_PARTIDAS];
    int n;
} BDPartidas;

void bdpartidas_init(BDPartidas *bd);
int bdpartidas_carregar_csv(BDPartidas *bd, const char *caminho);

void bdpartidas_aplicar_em_bdtimes(const BDPartidas *bdp, BDTimes *bdt);

void bdpartidas_listar_por_mandante_prefixo(const BDPartidas *bdp, const BDTimes *bdt, const char *prefixo);
void bdpartidas_listar_por_visitante_prefixo(const BDPartidas *bdp, const BDTimes *bdt, const char *prefixo);
void bdpartidas_listar_por_qualquer_prefixo(const BDPartidas *bdp, const BDTimes *bdt, const char *prefixo);

#endif
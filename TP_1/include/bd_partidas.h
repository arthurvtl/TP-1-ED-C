#ifndef BD_PARTIDAS_H
#define BD_PARTIDAS_H

#include <stddef.h>
#include "bd_times.h"

#define MAX_PARTIDAS 500  // Limite maximo de partidas armazenadas simultaneamente

typedef struct {
    int id;     // Identificador unico da partida
    int time1;  // Indice do time mandante na base de times
    int time2;  // Indice do time visitante na base de times
    int g1;     // Gols marcados pelo mandante
    int g2;     // Gols marcados pelo visitante
} Partida;

typedef struct {
    Partida partidas[MAX_PARTIDAS];  // Armazena todas as partidas carregadas
    int n;                           // Numero atual de partidas validas no vetor
} BDPartidas;

void bdpartidas_init(BDPartidas *bd);  // Inicializa a base deixando-a pronta para uso
int bdpartidas_carregar_csv(BDPartidas *bd, const char *caminho);  // Preenche a base lendo de um arquivo CSV

void bdpartidas_aplicar_em_bdtimes(const BDPartidas *bdp, BDTimes *bdt);  // Atualiza estatisticas dos times com as partidas

void bdpartidas_listar_por_mandante_prefixo(const BDPartidas *bdp, const BDTimes *bdt, const char *prefixo);  // Lista partidas filtrando pelo prefixo do mandante
void bdpartidas_listar_por_visitante_prefixo(const BDPartidas *bdp, const BDTimes *bdt, const char *prefixo);  // Lista partidas filtrando pelo prefixo do visitante
void bdpartidas_listar_por_qualquer_prefixo(const BDPartidas *bdp, const BDTimes *bdt, const char *prefixo);   // Lista partidas filtrando pelo prefixo em qualquer um dos times

#endif

#ifndef BD_TIMES_H
#define BD_TIMES_H

#include <stddef.h>

#define MAX_TIMES 64       // Capacidade maxima de times carregados
#define MAX_NOME_TIME 64   // Tamanho fixo do buffer de nome (com terminador)

typedef struct {
    int id;                         // Identificador unico conforme arquivo CSV
    char nome[MAX_NOME_TIME];       // Nome do time em UTF-8
    int v;                          // Vitorias acumuladas
    int e;                          // Empates acumulados
    int d;                          // Derrotas acumuladas
    int gm;                         // Gols marcados
    int gs;                         // Gols sofridos
} Time;

typedef struct {
    Time times[MAX_TIMES];          // Vetor fixo com os registros carregados
    int n;                          // Numero de times validos no vetor
} BDTimes;

void bdtimes_init(BDTimes *bd);  // Reseta a base para estado vazio
int bdtimes_carregar_csv(BDTimes *bd, const char *caminho);  // Carrega times a partir de um CSV (retorna quantidade lida)
Time* bdtimes_buscar_por_id(BDTimes *bd, int id);  // Busca time pelo identificador interno
int bdtimes_buscar_por_prefixo(BDTimes *bd, const char *prefixo, int *indices, int max_indices);  // Localiza todos os times com prefixo

void time_zerar_stats(Time *t);  // Limpa estatisticas (usado ao carregar CSV)
void time_acumular_partida(Time *t, int gols_feitos, int gols_sofridos);  // Atualiza estatisticas apos partida
int time_pontos(const Time *t);  // Calcula pontuacao (3V + 1E)
int time_saldo(const Time *t);   // Calcula saldo de gols

void bdtimes_imprimir_classificacao(const BDTimes *bd);  // Imprime tabela formatada

#endif

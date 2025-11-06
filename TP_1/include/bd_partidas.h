/**
 * Header: bd_partidas.h
 * 
 * Define as estruturas de dados e interfaces publicas para o gerenciamento
 * de partidas em um sistema de campeonato de futebol.
 * 
 * Este modulo oferece funcionalidades para:
 * - Armazenar informacoes de partidas (times, placares)
 * - Carregar partidas de arquivos CSV
 * - Aplicar resultados das partidas nas estatisticas dos times
 * - Listar partidas filtradas por time (mandante, visitante ou ambos)
 * 
 * Estruturas principais:
 * - Partida: Representa uma unica partida entre dois times
 * - BDPartidas: Base de dados em memoria contendo multiplas partidas
 * 
 * O sistema trabalha em conjunto com bd_times.h, atualizando as
 * estatisticas dos times baseado nos resultados das partidas.
 */

#ifndef BD_PARTIDAS_H
#define BD_PARTIDAS_H

#include <stddef.h>
#include "bd_times.h"

// Constantes de configuracao do sistema
#define MAX_PARTIDAS 500  // Limite maximo de partidas que podem ser armazenadas simultaneamente

/**
 * Estrutura que representa uma partida de futebol.
 * 
 * Armazena informacoes sobre um confronto entre dois times:
 * - IDs dos times participantes (mandante e visitante)
 * - Placar da partida (gols de cada time)
 * - Identificador unico da partida
 * 
 * Convencao:
 * - time1: Time mandante (joga em casa)
 * - time2: Time visitante
 * - g1: Gols do mandante
 * - g2: Gols do visitante
 */
typedef struct {
    int id;     // Identificador unico da partida (definido no arquivo CSV)
    int time1;  // ID do time mandante (referencia o campo 'id' na estrutura Time)
    int time2;  // ID do time visitante (referencia o campo 'id' na estrutura Time)
    int g1;     // Numero de gols marcados pelo time mandante
    int g2;     // Numero de gols marcados pelo time visitante
} Partida;

/**
 * Estrutura que representa o banco de dados de partidas em memoria.
 * 
 * Mantem um array de tamanho fixo com todas as partidas carregadas.
 * O campo 'n' indica quantos elementos do array sao validos.
 * 
 * Capacidade maxima: MAX_PARTIDAS (500 partidas)
 */
typedef struct {
    Partida partidas[MAX_PARTIDAS];  // Array de tamanho fixo contendo as partidas carregadas
    int n;                           // Numero de partidas validas atualmente no array (0 a MAX_PARTIDAS)
} BDPartidas;

// ========== Funcoes de gerenciamento da base de dados ==========

/**
 * Inicializa a estrutura BDPartidas para uso.
 * 
 * Deve ser chamada antes de qualquer operacao com a base de dados.
 * Reseta o contador de partidas para zero.
 * 
 * @param bd Ponteiro para a estrutura BDPartidas a ser inicializada
 */
void bdpartidas_init(BDPartidas *bd);

/**
 * Carrega partidas de um arquivo CSV.
 * 
 * Le um arquivo CSV no formato "ID,Time1ID,Time2ID,Gols1,Gols2" e carrega
 * as partidas na base de dados. A primeira linha do arquivo (cabecalho)
 * e descartada.
 * 
 * Formato esperado do arquivo:
 * ID,Time1ID,Time2ID,Gols1,Gols2
 * 0,5,3,2,1
 * 1,7,2,0,0
 * ...
 * 
 * @param bd Ponteiro para a estrutura BDPartidas onde as partidas serao armazenadas
 * @param caminho Caminho do arquivo CSV a ser lido
 * @return Numero de partidas carregadas com sucesso, ou 0 se houver erro ao abrir
 */
int bdpartidas_carregar_csv(BDPartidas *bd, const char *caminho);

// ========== Funcoes de processamento ==========

/**
 * Aplica os resultados das partidas nas estatisticas dos times.
 * 
 * Para cada partida carregada, esta funcao:
 * - Busca os dois times participantes na base de times
 * - Atualiza as estatisticas de cada time (vitorias, empates, derrotas, gols)
 * 
 * Esta funcao deve ser chamada apos carregar os times e partidas,
 * para calcular a classificacao do campeonato.
 * 
 * Se um time referenciado em uma partida nao existir na base de times,
 * um aviso e emitido e a partida e ignorada.
 * 
 * @param bdp Ponteiro para a estrutura BDPartidas contendo as partidas
 * @param bdt Ponteiro para a estrutura BDTimes cujas estatisticas serao atualizadas
 */
void bdpartidas_aplicar_em_bdtimes(const BDPartidas *bdp, BDTimes *bdt);

// ========== Funcoes de listagem e consulta ==========

/**
 * Lista partidas filtrando pelo prefixo do time mandante.
 * 
 * Exibe todas as partidas onde o nome do time mandante (time1)
 * comeca com o prefixo especificado. A busca e case-insensitive.
 * 
 * Formato de saida:
 * | ID | Time1 | Placar | Time2 |
 * 
 * @param bdp Ponteiro para a estrutura BDPartidas contendo as partidas
 * @param bdt Ponteiro para a estrutura BDTimes para buscar nomes dos times
 * @param prefixo Prefixo do nome do time mandante a buscar (ex: "Fla")
 */
void bdpartidas_listar_por_mandante_prefixo(const BDPartidas *bdp, const BDTimes *bdt, const char *prefixo);

/**
 * Lista partidas filtrando pelo prefixo do time visitante.
 * 
 * Exibe todas as partidas onde o nome do time visitante (time2)
 * comeca com o prefixo especificado. A busca e case-insensitive.
 * 
 * Formato de saida:
 * | ID | Time1 | Placar | Time2 |
 * 
 * @param bdp Ponteiro para a estrutura BDPartidas contendo as partidas
 * @param bdt Ponteiro para a estrutura BDTimes para buscar nomes dos times
 * @param prefixo Prefixo do nome do time visitante a buscar (ex: "Pal")
 */
void bdpartidas_listar_por_visitante_prefixo(const BDPartidas *bdp, const BDTimes *bdt, const char *prefixo);

/**
 * Lista partidas filtrando pelo prefixo de qualquer time.
 * 
 * Exibe todas as partidas onde o nome do time mandante OU visitante
 * comeca com o prefixo especificado. A busca e case-insensitive.
 * 
 * Esta funcao e util para ver todas as partidas de um time especifico,
 * independente de ter jogado como mandante ou visitante.
 * 
 * Formato de saida:
 * | ID | Time1 | Placar | Time2 |
 * 
 * @param bdp Ponteiro para a estrutura BDPartidas contendo as partidas
 * @param bdt Ponteiro para a estrutura BDTimes para buscar nomes dos times
 * @param prefixo Prefixo do nome do time a buscar (ex: "Cor")
 */
void bdpartidas_listar_por_qualquer_prefixo(const BDPartidas *bdp, const BDTimes *bdt, const char *prefixo);

#endif

/**
 * Header: bd_times.h
 * 
 * Define as estruturas de dados e interfaces publicas para o gerenciamento
 * de times em um sistema de campeonato de futebol.
 * 
 * Este modulo oferece funcionalidades para:
 * - Armazenar informacoes de times (ID, nome, estatisticas)
 * - Carregar times de arquivos CSV
 * - Buscar times por ID ou prefixo do nome
 * - Acumular estatisticas de partidas
 * - Calcular pontuacao e saldo de gols
 * - Imprimir e exportar tabelas de classificacao
 * 
 * Estruturas principais:
 * - Time: Representa um unico time com suas estatisticas
 * - BDTimes: Base de dados em memoria contendo multiplos times
 */

#ifndef BD_TIMES_H
#define BD_TIMES_H

#include <stddef.h>

// Constantes de configuracao do sistema
#define MAX_TIMES 64       // Capacidade maxima de times que podem ser carregados simultaneamente
#define MAX_NOME_TIME 64   // Tamanho maximo do buffer para nome do time (incluindo terminador nulo)

/**
 * Estrutura que representa um time de futebol.
 * 
 * Armazena todas as informacoes relevantes sobre um time incluindo:
 * - Identificacao (ID e nome)
 * - Estatisticas acumuladas (vitorias, empates, derrotas)
 * - Gols (marcados e sofridos)
 * 
 * As estatisticas sao acumuladas ao processar partidas usando
 * a funcao time_acumular_partida().
 */
typedef struct {
    int id;                         // Identificador unico do time (definido no arquivo CSV)
    char nome[MAX_NOME_TIME];       // Nome do time codificado em UTF-8
    int v;                          // Total de vitorias acumuladas
    int e;                          // Total de empates acumulados
    int d;                          // Total de derrotas acumuladas
    int gm;                         // Total de gols marcados pelo time
    int gs;                         // Total de gols sofridos pelo time
} Time;

/**
 * Estrutura que representa o banco de dados de times em memoria.
 * 
 * Mantem um array de tamanho fixo com todos os times carregados.
 * O campo 'n' indica quantos elementos do array sao validos.
 * 
 * Capacidade maxima: MAX_TIMES (64 times)
 */
typedef struct {
    Time times[MAX_TIMES];          // Array de tamanho fixo contendo os times carregados
    int n;                          // Numero de times validos atualmente no array (0 a MAX_TIMES)
} BDTimes;

// ========== Funcoes de gerenciamento da base de dados ==========

/**
 * Inicializa a estrutura BDTimes para uso.
 * 
 * Deve ser chamada antes de qualquer operacao com a base de dados.
 * Reseta o contador de times para zero.
 * 
 * @param bd Ponteiro para a estrutura BDTimes a ser inicializada
 */
void bdtimes_init(BDTimes *bd);

/**
 * Carrega times de um arquivo CSV.
 * 
 * Le um arquivo CSV no formato "ID,Nome" e carrega os times na base de dados.
 * A primeira linha do arquivo (cabecalho) e descartada.
 * Todos os times sao carregados com estatisticas zeradas.
 * 
 * Formato esperado do arquivo:
 * ID,Nome
 * 0,Flamengo
 * 1,Palmeiras
 * ...
 * 
 * @param bd Ponteiro para a estrutura BDTimes onde os times serao armazenados
 * @param caminho Caminho do arquivo CSV a ser lido
 * @return Numero de times carregados com sucesso, ou 0 se houver erro ao abrir
 */
int bdtimes_carregar_csv(BDTimes *bd, const char *caminho);

/**
 * Busca um time pelo seu ID unico.
 * 
 * Realiza uma busca linear na base de dados procurando um time
 * com o ID especificado.
 * 
 * @param bd Ponteiro para a estrutura BDTimes onde buscar
 * @param id ID do time procurado
 * @return Ponteiro para o Time encontrado, ou NULL se nao existir
 */
Time* bdtimes_buscar_por_id(BDTimes *bd, int id);

/**
 * Busca times cujo nome comeca com um prefixo.
 * 
 * Realiza uma busca case-insensitive por times cujo nome comeca
 * com o prefixo especificado. Os indices dos times encontrados
 * sao armazenados no array fornecido.
 * 
 * @param bd Ponteiro para a estrutura BDTimes onde buscar
 * @param prefixo String com o prefixo a buscar (ex: "Fla")
 * @param indices Array onde os indices dos times encontrados serao armazenados
 * @param max_indices Tamanho maximo do array indices
 * @return Total de times encontrados (pode exceder max_indices)
 */
int bdtimes_buscar_por_prefixo(BDTimes *bd, const char *prefixo, int *indices, int max_indices);

// ========== Funcoes para manipulacao de times individuais ==========

/**
 * Zera todas as estatisticas de um time.
 * 
 * Reseta vitorias, empates, derrotas, gols marcados e gols sofridos
 * para zero. Util ao carregar times do CSV antes de processar partidas.
 * 
 * @param t Ponteiro para o time cujas estatisticas serao zeradas
 */
void time_zerar_stats(Time *t);

/**
 * Acumula o resultado de uma partida nas estatisticas do time.
 * 
 * Atualiza os contadores do time baseado no resultado de uma partida:
 * - Incrementa vitorias, empates ou derrotas conforme o resultado
 * - Acumula gols marcados e sofridos
 * 
 * @param t Ponteiro para o time a ser atualizado
 * @param gols_feitos Numero de gols que o time marcou
 * @param gols_sofridos Numero de gols que o time sofreu
 */
void time_acumular_partida(Time *t, int gols_feitos, int gols_sofridos);

/**
 * Calcula o total de pontos ganhos por um time.
 * 
 * A pontuacao segue as regras do futebol:
 * - 3 pontos por vitoria
 * - 1 ponto por empate
 * - 0 pontos por derrota
 * 
 * @param t Ponteiro para o time
 * @return Total de pontos ganhos (3*V + E)
 */
int time_pontos(const Time *t);

/**
 * Calcula o saldo de gols de um time.
 * 
 * O saldo e a diferenca entre gols marcados e gols sofridos.
 * Valores positivos indicam mais gols marcados que sofridos.
 * 
 * @param t Ponteiro para o time
 * @return Saldo de gols (GM - GS)
 */
int time_saldo(const Time *t);

// ========== Funcoes de exibicao ==========

/**
 * Imprime a tabela de classificacao na tela e exporta para CSV.
 * 
 * Exibe todos os times em formato de tabela visual com colunas
 * alinhadas. Alem disso, cria/sobrescreve o arquivo "bd_classificacao.csv"
 * com os mesmos dados em formato CSV com separador "|".
 * 
 * Os times sao ordenados por ID de forma crescente.
 * 
 * Colunas: ID | Time | V | E | D | GM | GS | S | PG
 * 
 * @param bd Ponteiro para a estrutura BDTimes contendo os times
 */
void bdtimes_imprimir_classificacao(const BDTimes *bd);

#endif

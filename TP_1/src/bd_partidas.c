/**
 * Modulo: bd_partidas.c
 * 
 * Este modulo implementa as funcionalidades relacionadas ao gerenciamento de partidas
 * em um sistema de campeonato de futebol. Fornece estruturas e funcoes para:
 * - Carregar partidas de um arquivo CSV
 * - Armazenar informacoes de confrontos entre times
 * - Aplicar resultados das partidas nas estatisticas dos times
 * - Listar partidas filtradas por time (mandante, visitante ou ambos)
 * 
 * O sistema usa uma base de dados em memoria (BDPartidas) que armazena ate MAX_PARTIDAS
 * partidas simultaneamente. Cada partida conecta dois times e registra o placar.
 * 
 * Este modulo trabalha em conjunto com bd_times.c, atualizando as estatisticas
 * dos times baseado nos resultados das partidas carregadas.
 */

#include "bd_partidas.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * Inicializa a estrutura de banco de dados de partidas.
 * 
 * Prepara a estrutura BDPartidas para uso, zerando o contador de partidas.
 * Deve ser chamada antes de qualquer outra operacao com a base de dados.
 * 
 * @param bd Ponteiro para a estrutura BDPartidas a ser inicializada
 */
void bdpartidas_init(BDPartidas *bd) {
    // Inicializa com zero registros carregados
    // O array de partidas existe mas nenhum elemento e considerado valido ainda
    bd->n = 0;
}

/**
 * Remove caracteres de nova linha do final de uma string.
 * 
 * Esta funcao auxiliar limpa o final de strings lidas com fgets(),
 * removendo caracteres '\n' e '\r' que geralmente ficam anexados.
 * A string e modificada in-place.
 * 
 * @param s String a ser limpa (modificada diretamente)
 */
static void chomp(char *s) {
    // Obtem o tamanho atual da string
    size_t n = strlen(s);
    
    // Remove caracteres de nova linha do final da string
    // Percorre de tras para frente enquanto encontrar '\n' ou '\r'
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[--n] = '\0';  // Substitui o caractere por terminador nulo
    }
}

/**
 * Faz o parsing de uma linha do arquivo CSV de partidas.
 * 
 * Esta funcao auxiliar processa uma linha no formato "ID,Time1ID,Time2ID,Gols1,Gols2"
 * e extrai os cinco campos numericos. A linha e modificada no processo (strtok).
 * 
 * Formato esperado: "ID,Time1ID,Time2ID,Gols1,Gols2"
 * Exemplo: "0,5,3,2,1" representa:
 *   - Partida ID 0
 *   - Time 5 (mandante) x Time 3 (visitante)
 *   - Placar: 2 x 1
 * 
 * @param linha String contendo a linha a ser processada (sera modificada)
 * @param out Ponteiro para a estrutura Partida onde os dados serao armazenados
 * @return 1 se o parsing foi bem sucedido, 0 se houver erro
 */
static int parse_partida_linha(char *linha, Partida *out) {
    // Remove caracteres de nova linha e espacos em branco
    chomp(linha);
    str_trim(linha);

    char *tok;      // Ponteiro para o token atual
    int vals[5];    // Array temporario para armazenar os 5 valores
    int i = 0;      // Indice atual no array de valores

    // Extrai o primeiro campo (ID da partida)
    tok = strtok(linha, ",");
    
    // Loop para extrair todos os 5 campos
    while (tok && i < 5) {
        // Limpa espacos em branco do campo
        str_trim(tok);
        chomp(tok);
        
        // Converte o campo de string para inteiro
        int v;
        if (!safe_atoi(tok, &v)) {
            // Campo invalido (nao e um numero valido)
            return 0;
        }
        
        // Armazena o valor no array temporario
        vals[i++] = v;
        
        // Extrai o proximo campo
        tok = strtok(NULL, ",");
    }
    
    // Verifica se exatamente 5 campos foram extraidos
    if (i != 5) return 0;  // Linha incompleta ou com campos extras

    // Popula a estrutura Partida com os valores extraidos
    out->id    = vals[0];  // ID da partida
    out->time1 = vals[1];  // ID do time mandante
    out->time2 = vals[2];  // ID do time visitante
    out->g1    = vals[3];  // Gols do mandante
    out->g2    = vals[4];  // Gols do visitante
    
    return 1;  // Sucesso
}

/**
 * Carrega partidas de um arquivo CSV para a base de dados.
 * 
 * Le um arquivo CSV contendo partidas no formato "ID,Time1ID,Time2ID,Gols1,Gols2",
 * uma linha por partida. A primeira linha do arquivo (cabecalho) e descartada.
 * 
 * Formato do arquivo:
 * ID,Time1ID,Time2ID,Gols1,Gols2
 * 0,5,3,2,1
 * 1,7,2,0,0
 * ...
 * 
 * As partidas carregadas ainda nao afetam as estatisticas dos times.
 * Para aplicar os resultados, deve-se chamar bdpartidas_aplicar_em_bdtimes().
 * 
 * @param bd Ponteiro para a estrutura BDPartidas onde as partidas serao carregadas
 * @param caminho Caminho do arquivo CSV a ser lido
 * @return Numero de partidas carregadas com sucesso, ou 0 se houver erro ao abrir o arquivo
 */
int bdpartidas_carregar_csv(BDPartidas *bd, const char *caminho) {
    // Abre o arquivo CSV para leitura
    FILE *f = fopen(caminho, "r");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo de partidas: %s\n", caminho);
        return 0;
    }
    
    char buf[512];  // Buffer para ler cada linha do arquivo

    // Descarta a primeira linha (cabecalho com "ID,Time1ID,Time2ID,Gols1,Gols2")
    if (!fgets(buf, sizeof(buf), f)) {
        fclose(f);
        fprintf(stderr, "Arquivo de partidas vazio ou invalido: %s\n", caminho);
        return 0;
    }

    int count = 0;  // Contador de partidas carregadas com sucesso
    
    // Le cada linha do arquivo ate o fim
    while (fgets(buf, sizeof(buf), f)) {
        // Verifica se ja atingiu o limite maximo de partidas
        if (bd->n >= MAX_PARTIDAS) {
            fprintf(stderr, "Limite de partidas atingido (%d)\n", MAX_PARTIDAS);
            break;
        }
        
        // Cria uma copia da linha para nao modificar o buffer original
        char linha[512];
        strncpy(linha, buf, sizeof(linha) - 1);
        linha[sizeof(linha) - 1] = '\0';

        // Faz o parsing da linha extraindo todos os campos
        Partida p;
        if (!parse_partida_linha(linha, &p)) {
            // Se o parsing falhar, ignora esta linha e continua
            fprintf(stderr, "Linha de partida ignorada (parse falhou): %s", buf);
            continue;
        }
        
        // Adiciona a partida ao array e incrementa o contador
        bd->partidas[bd->n++] = p;
        count++;
    }
    
    // Fecha o arquivo apos terminar a leitura
    fclose(f);
    
    return count;  // Retorna quantas partidas foram carregadas
}

/**
 * Aplica os resultados das partidas nas estatisticas dos times.
 * 
 * Para cada partida carregada, esta funcao:
 * 1. Busca os dois times participantes na base de times
 * 2. Atualiza as estatisticas de vitoria/empate/derrota para cada time
 * 3. Acumula gols marcados e sofridos
 * 
 * Esta funcao deve ser chamada apos carregar times e partidas, para que
 * a tabela de classificacao reflita os resultados do campeonato.
 * 
 * Comportamento em caso de erro:
 * - Se um time referenciado nao existir, um aviso e emitido e a partida e ignorada
 * - As demais partidas continuam sendo processadas normalmente
 * 
 * @param bdp Ponteiro para a estrutura BDPartidas contendo as partidas
 * @param bdt Ponteiro para a estrutura BDTimes cujas estatisticas serao atualizadas
 */
void bdpartidas_aplicar_em_bdtimes(const BDPartidas *bdp, BDTimes *bdt) {
    // Percorre todas as partidas carregadas
    for (int i = 0; i < bdp->n; i++) {
        const Partida *p = &bdp->partidas[i];
        
        // Busca os dois times participantes da partida
        Time *t1 = bdtimes_buscar_por_id(bdt, p->time1);  // Time mandante
        Time *t2 = bdtimes_buscar_por_id(bdt, p->time2);  // Time visitante
        
        // Verifica se ambos os times foram encontrados
        if (!t1 || !t2) {
            // Um ou ambos os times nao existem na base de dados
            fprintf(stderr, "Aviso: partida %d referencia time inexistente (%d,%d)\n", 
                    p->id, p->time1, p->time2);
            continue;  // Pula esta partida e continua com a proxima
        }
        
        // Atualiza as estatisticas do time mandante (t1)
        // Para t1: gols feitos = g1, gols sofridos = g2
        time_acumular_partida(t1, p->g1, p->g2);
        
        // Atualiza as estatisticas do time visitante (t2)
        // Para t2: gols feitos = g2, gols sofridos = g1 (perspectiva invertida)
        time_acumular_partida(t2, p->g2, p->g1);
    }
}

/**
 * Busca o nome de um time pelo seu ID.
 * 
 * Funcao auxiliar usada pelas funcoes de listagem de partidas.
 * Procura um time na base e retorna seu nome.
 * 
 * @param bdt Ponteiro para a estrutura BDTimes onde buscar
 * @param id ID do time procurado
 * @return Nome do time encontrado, ou "(desconhecido)" se nao existir
 */
static const char* nome_do_time(const BDTimes *bdt, int id) {
    // Busca linear por um time com o ID especificado
    for (int i = 0; i < bdt->n; i++) {
        if (bdt->times[i].id == id) {
            return bdt->times[i].nome;
        }
    }
    
    // Time nao encontrado
    return "(desconhecido)";
}

/**
 * Lista partidas filtrando pelo prefixo do time mandante.
 * 
 * Exibe todas as partidas onde o nome do time mandante (time1) comeca
 * com o prefixo especificado. A busca e case-insensitive (ignora maiusculas/minusculas).
 * 
 * Formato de saida:
 * | ID | Time1 | Placar | Time2 |
 * | 5  | Flamengo | 3 x 1 | Santos |
 * 
 * Se nenhuma partida for encontrada, uma mensagem informativa e exibida.
 * 
 * @param bdp Ponteiro para a estrutura BDPartidas contendo as partidas
 * @param bdt Ponteiro para a estrutura BDTimes para buscar nomes dos times
 * @param prefixo Prefixo do nome do time mandante a buscar (ex: "Fla")
 */
void bdpartidas_listar_por_mandante_prefixo(const BDPartidas *bdp, const BDTimes *bdt, const char *prefixo) {
    int count = 0;  // Contador de partidas encontradas
    
    // Imprime o cabecalho da tabela
    printf("| ID | Time1 |  | Time2 |\n");
    printf("|----|-------|--|-------|\n");
    
    // Percorre todas as partidas carregadas
    for (int i = 0; i < bdp->n; i++) {
        const Partida *p = &bdp->partidas[i];
        
        // Busca o nome do time mandante
        const char *n1 = nome_do_time(bdt, p->time1);
        
        // Verifica se o nome do mandante comeca com o prefixo (case-insensitive)
        if (str_starts_with_case_insensitive(n1, prefixo)) {
            // Partida encontrada! Busca o nome do visitante e imprime a linha
            const char *n2 = nome_do_time(bdt, p->time2);
            printf("| %d | %s | %d x %d | %s |\n", p->id, n1, p->g1, p->g2, n2);
            count++;
        }
    }
    
    // Se nenhuma partida foi encontrada, informa ao usuario
    if (count == 0) {
        printf("Nenhuma partida encontrada para mandante com prefixo: %s\n", prefixo);
    }
}

/**
 * Lista partidas filtrando pelo prefixo do time visitante.
 * 
 * Exibe todas as partidas onde o nome do time visitante (time2) comeca
 * com o prefixo especificado. A busca e case-insensitive (ignora maiusculas/minusculas).
 * 
 * Formato de saida:
 * | ID | Time1 | Placar | Time2 |
 * | 7  | Corinthians | 2 x 2 | Palmeiras |
 * 
 * Se nenhuma partida for encontrada, uma mensagem informativa e exibida.
 * 
 * @param bdp Ponteiro para a estrutura BDPartidas contendo as partidas
 * @param bdt Ponteiro para a estrutura BDTimes para buscar nomes dos times
 * @param prefixo Prefixo do nome do time visitante a buscar (ex: "Pal")
 */
void bdpartidas_listar_por_visitante_prefixo(const BDPartidas *bdp, const BDTimes *bdt, const char *prefixo) {
    int count = 0;  // Contador de partidas encontradas
    
    // Imprime o cabecalho da tabela
    printf("| ID | Time1 |  | Time2 |\n");
    printf("|----|-------|--|-------|\n");
    
    // Percorre todas as partidas carregadas
    for (int i = 0; i < bdp->n; i++) {
        const Partida *p = &bdp->partidas[i];
        
        // Busca o nome do time visitante
        const char *n2 = nome_do_time(bdt, p->time2);
        
        // Verifica se o nome do visitante comeca com o prefixo (case-insensitive)
        if (str_starts_with_case_insensitive(n2, prefixo)) {
            // Partida encontrada! Busca o nome do mandante e imprime a linha
            const char *n1 = nome_do_time(bdt, p->time1);
            printf("| %d | %s | %d x %d | %s |\n", p->id, n1, p->g1, p->g2, n2);
            count++;
        }
    }
    
    // Se nenhuma partida foi encontrada, informa ao usuario
    if (count == 0) {
        printf("Nenhuma partida encontrada para visitante com prefixo: %s\n", prefixo);
    }
}

/**
 * Lista partidas filtrando pelo prefixo de qualquer time (mandante ou visitante).
 * 
 * Exibe todas as partidas onde o nome do time mandante OU visitante comeca
 * com o prefixo especificado. A busca e case-insensitive (ignora maiusculas/minusculas).
 * 
 * Esta funcao e util para ver todas as partidas de um time especifico,
 * independente de ter jogado como mandante ou visitante.
 * 
 * Formato de saida:
 * | ID | Time1 | Placar | Time2 |
 * | 3  | Santos | 1 x 2 | Corinthians |
 * | 8  | Corinthians | 3 x 0 | Botafogo |
 * 
 * Se nenhuma partida for encontrada, uma mensagem informativa e exibida.
 * 
 * @param bdp Ponteiro para a estrutura BDPartidas contendo as partidas
 * @param bdt Ponteiro para a estrutura BDTimes para buscar nomes dos times
 * @param prefixo Prefixo do nome do time a buscar (ex: "Cor")
 */
void bdpartidas_listar_por_qualquer_prefixo(const BDPartidas *bdp, const BDTimes *bdt, const char *prefixo) {
    int count = 0;  // Contador de partidas encontradas
    
    // Imprime o cabecalho da tabela
    printf("| ID | Time1 |  | Time2 |\n");
    printf("|----|-------|--|-------|\n");
    
    // Percorre todas as partidas carregadas
    for (int i = 0; i < bdp->n; i++) {
        const Partida *p = &bdp->partidas[i];
        
        // Busca os nomes de ambos os times
        const char *n1 = nome_do_time(bdt, p->time1);  // Mandante
        const char *n2 = nome_do_time(bdt, p->time2);  // Visitante
        
        // Verifica se qualquer um dos nomes comeca com o prefixo (case-insensitive)
        if (str_starts_with_case_insensitive(n1, prefixo) || 
            str_starts_with_case_insensitive(n2, prefixo)) {
            // Partida encontrada! Imprime a linha com os dados completos
            printf("| %d | %s | %d x %d | %s |\n", p->id, n1, p->g1, p->g2, n2);
            count++;
        }
    }
    
    // Se nenhuma partida foi encontrada, informa ao usuario
    if (count == 0) {
        printf("Nenhuma partida encontrada para mandante ou visitante com prefixo: %s\n", prefixo);
    }
}

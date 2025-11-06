/**
 * Modulo: bd_times.c
 * 
 * Este modulo implementa as funcionalidades relacionadas ao gerenciamento de times
 * em um sistema de campeonato de futebol. Fornece estruturas e funcoes para:
 * - Carregar times de um arquivo CSV
 * - Buscar times por ID ou prefixo do nome
 * - Acumular estatisticas de partidas (vitorias, empates, derrotas, gols)
 * - Calcular pontuacao e saldo de gols
 * - Imprimir e exportar a tabela de classificacao
 * 
 * O sistema usa uma base de dados em memoria (BDTimes) que armazena ate MAX_TIMES
 * times simultaneamente. Cada time possui um ID unico, nome e estatisticas acumuladas.
 */

#include "bd_times.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

/**
 * Remove caracteres de nova linha deixados por fgets.
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
 * Inicializa a estrutura de banco de dados de times.
 * 
 * Prepara a estrutura BDTimes para uso, zerando o contador de times.
 * Deve ser chamada antes de qualquer outra operacao com a base de dados.
 * 
 * @param bd Ponteiro para a estrutura BDTimes a ser inicializada
 */
void bdtimes_init(BDTimes *bd) {
    // Inicializa com zero registros carregados
    // O array de times existe mas nenhum elemento e considerado valido ainda
    bd->n = 0;
}

/**
 * Zera todas as estatisticas acumuladas de um time.
 * 
 * Esta funcao reseta os contadores de vitorias, empates, derrotas,
 * gols marcados e gols sofridos para zero. Util ao carregar times
 * de um arquivo CSV, garantindo que comecem sem estatisticas.
 * 
 * @param t Ponteiro para o time cujas estatisticas serao zeradas
 */
void time_zerar_stats(Time *t) {
    // Limpa todas as estatisticas acumuladas
    // Usado ao carregar times do CSV antes de aplicar as partidas
    t->v = 0;   // Zera vitorias
    t->e = 0;   // Zera empates
    t->d = 0;   // Zera derrotas
    t->gm = 0;  // Zera gols marcados
    t->gs = 0;  // Zera gols sofridos
}

/**
 * Acumula o resultado de uma partida nas estatisticas do time.
 * 
 * Atualiza os contadores do time baseado no resultado de uma unica partida:
 * - Incrementa vitorias se o time fez mais gols
 * - Incrementa empates se ambos fizeram o mesmo numero de gols
 * - Incrementa derrotas se o time sofreu mais gols
 * - Acumula gols marcados e sofridos
 * 
 * Esta funcao e chamada uma vez para cada time que participou de uma partida.
 * 
 * @param t Ponteiro para o time que teve estatisticas atualizadas
 * @param gols_feitos Numero de gols que o time marcou na partida
 * @param gols_sofridos Numero de gols que o time sofreu na partida
 */
void time_acumular_partida(Time *t, int gols_feitos, int gols_sofridos) {
    // Acumula os gols marcados e sofridos
    t->gm += gols_feitos;
    t->gs += gols_sofridos;
    
    // Determina o resultado da partida e atualiza o contador apropriado
    if (gols_feitos > gols_sofridos) {
        // Time venceu a partida
        t->v++;
    } else if (gols_feitos == gols_sofridos) {
        // Partida terminou empatada
        t->e++;
    } else {
        // Time perdeu a partida
        t->d++;
    }
}

/**
 * Calcula o numero total de pontos ganhos por um time.
 * 
 * No futebol, a pontuacao e calculada como:
 * - 3 pontos por vitoria
 * - 1 ponto por empate
 * - 0 pontos por derrota
 * 
 * @param t Ponteiro para o time cuja pontuacao sera calculada
 * @return Numero total de pontos ganhos pelo time
 */
int time_pontos(const Time *t) {
    // Formula: Pontos = (3 * Vitorias) + (1 * Empates)
    return t->v * 3 + t->e;
}

/**
 * Calcula o saldo de gols de um time.
 * 
 * O saldo de gols e a diferenca entre gols marcados e gols sofridos.
 * Um saldo positivo indica mais gols marcados que sofridos.
 * Um saldo negativo indica mais gols sofridos que marcados.
 * 
 * @param t Ponteiro para o time cujo saldo sera calculado
 * @return Saldo de gols (gols marcados - gols sofridos)
 */
int time_saldo(const Time *t) {
    // Formula: Saldo = Gols Marcados - Gols Sofridos
    return t->gm - t->gs;
}

/**
 * Faz o parsing de uma linha do arquivo CSV de times.
 * 
 * Esta funcao auxiliar processa uma linha no formato "ID,Nome" e extrai
 * os dois campos separadamente. A linha e modificada no processo (strtok).
 * 
 * Formato esperado: "ID,Nome"
 * Exemplo: "0,Flamengo"
 * 
 * @param linha String contendo a linha a ser processada (sera modificada)
 * @param id Ponteiro onde o ID extraido sera armazenado
 * @param nome Buffer onde o nome extraido sera copiado (deve ter tamanho MAX_NOME_TIME)
 * @return 1 se o parsing foi bem sucedido, 0 se houver erro
 */
static int parse_time_linha(char *linha, int *id, char *nome) {
    // Remove caracteres de nova linha e espacos em branco
    chomp(linha);
    str_trim(linha);

    // Extrai o primeiro campo (ID)
    char *tok = strtok(linha, ",");
    if (!tok) return 0;  // Linha vazia ou sem virgula
    
    // Converte o ID de string para inteiro
    int tmp_id;
    if (!safe_atoi(tok, &tmp_id)) return 0;  // ID invalido (nao e numero)

    // Extrai o segundo campo (Nome)
    tok = strtok(NULL, ",");
    if (!tok) return 0;  // Falta o campo nome
    
    // Limpa espacos em branco do nome
    str_trim(tok);
    chomp(tok);

    // Copia os valores extraidos para os parametros de saida
    *id = tmp_id;
    strncpy(nome, tok, MAX_NOME_TIME - 1);
    nome[MAX_NOME_TIME - 1] = '\0';  // Garante terminacao nula
    
    return 1;  // Sucesso
}

/**
 * Carrega times de um arquivo CSV para a base de dados.
 * 
 * Le um arquivo CSV contendo times no formato "ID,Nome", uma linha por time.
 * A primeira linha do arquivo (cabecalho) e descartada.
 * 
 * Formato do arquivo:
 * ID,Nome
 * 0,Flamengo
 * 1,Palmeiras
 * ...
 * 
 * Todos os times sao carregados com estatisticas zeradas. As estatisticas
 * serao acumuladas posteriormente ao processar as partidas.
 * 
 * @param bd Ponteiro para a estrutura BDTimes onde os times serao carregados
 * @param caminho Caminho do arquivo CSV a ser lido
 * @return Numero de times carregados com sucesso, ou 0 se houver erro ao abrir o arquivo
 */
int bdtimes_carregar_csv(BDTimes *bd, const char *caminho) {
    // Abre o arquivo CSV para leitura
    FILE *f = fopen(caminho, "r");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo de times: %s\n", caminho);
        return 0;
    }
    
    char buf[256];  // Buffer para ler cada linha do arquivo

    // Descarta a primeira linha (cabecalho com "ID,Nome")
    if (!fgets(buf, sizeof(buf), f)) {
        fclose(f);
        fprintf(stderr, "Arquivo de times vazio ou invalido: %s\n", caminho);
        return 0;
    }

    int count = 0;  // Contador de times carregados com sucesso
    
    // Le cada linha do arquivo ate o fim
    while (fgets(buf, sizeof(buf), f)) {
        // Verifica se ja atingiu o limite maximo de times
        if (bd->n >= MAX_TIMES) {
            fprintf(stderr, "Limite de times atingido (%d)\n", MAX_TIMES);
            break;
        }
        
        // Cria uma copia da linha para nao modificar o buffer original
        char linha[256];
        strncpy(linha, buf, sizeof(linha) - 1);
        linha[sizeof(linha) - 1] = '\0';

        // Faz o parsing da linha extraindo ID e nome
        int id;
        char nome[MAX_NOME_TIME];
        if (!parse_time_linha(linha, &id, nome)) {
            // Se o parsing falhar, ignora esta linha e continua
            fprintf(stderr, "Linha de time ignorada (parse falhou): %s", buf);
            continue;
        }
        
        // Cria uma nova estrutura Time com os dados extraidos
        Time t;
        t.id = id;
        strncpy(t.nome, nome, MAX_NOME_TIME);
        t.nome[MAX_NOME_TIME - 1] = '\0';
        
        // Inicializa as estatisticas com zero
        // Elas serao atualizadas posteriormente ao processar partidas
        time_zerar_stats(&t);
        
        // Adiciona o time ao array e incrementa o contador
        bd->times[bd->n++] = t;
        count++;
    }
    
    // Fecha o arquivo apos terminar a leitura
    fclose(f);
    
    return count;  // Retorna quantos times foram carregados
}

/**
 * Busca um time pelo seu ID.
 * 
 * Procura linearmente na base de dados por um time com o ID especificado.
 * Esta busca e eficiente para bases pequenas (ate MAX_TIMES = 64 times).
 * 
 * @param bd Ponteiro para a estrutura BDTimes onde buscar
 * @param id ID do time procurado
 * @return Ponteiro para o Time encontrado, ou NULL se nao existir
 */
Time* bdtimes_buscar_por_id(BDTimes *bd, int id) {
    // Percorre todos os times carregados
    for (int i = 0; i < bd->n; i++) {
        if (bd->times[i].id == id) {
            // Time encontrado! Retorna um ponteiro para ele
            return &bd->times[i];
        }
    }
    
    // Time nao encontrado
    return NULL;
}

/**
 * Busca times cujo nome comeca com um prefixo especifico.
 * 
 * Realiza uma busca case-insensitive (ignora maiusculas/minusculas) por todos
 * os times cujo nome comeca com o prefixo especificado.
 * 
 * Os indices dos times encontrados sao armazenados no array 'indices'.
 * Se houver mais times que o limite max_indices, apenas os primeiros serao
 * armazenados, mas o total encontrado sera retornado.
 * 
 * Exemplo: prefixo "Fla" encontraria "Flamengo", "Fluminense", etc.
 * 
 * @param bd Ponteiro para a estrutura BDTimes onde buscar
 * @param prefixo String com o prefixo a buscar
 * @param indices Array onde os indices dos times encontrados serao armazenados
 * @param max_indices Tamanho maximo do array indices
 * @return Total de times encontrados (pode ser maior que max_indices)
 */
int bdtimes_buscar_por_prefixo(BDTimes *bd, const char *prefixo, int *indices, int max_indices) {
    int found = 0;  // Contador de times encontrados
    
    // Percorre todos os times carregados
    for (int i = 0; i < bd->n; i++) {
        // Verifica se o nome do time comeca com o prefixo (case-insensitive)
        if (str_starts_with_case_insensitive(bd->times[i].nome, prefixo)) {
            // Time encontrado!
            
            // Armazena o indice apenas se ainda ha espaco no array
            if (found < max_indices) {
                indices[found] = i;
            }
            
            // Incrementa o contador total (mesmo se nao coube no array)
            found++;
        }
    }
    
    return found;  // Retorna o total de times encontrados
}

/**
 * Exporta a tabela de classificacao para um arquivo CSV com formatacao alinhada.
 * 
 * Esta funcao cria/sobrescreve o arquivo "bd_classificacao.csv" contendo todos
 * os times ordenados por ID, com colunas separadas por "|" e alinhadas para
 * facilitar a leitura visual.
 * 
 * Formato do arquivo:
 * - Cabecalho com nomes das colunas
 * - Linha separadora com hifens
 * - Uma linha por time com todos os dados estatisticos
 * - Todas as colunas alinhadas usando espacos
 * 
 * @param bd Ponteiro para a estrutura BDTimes contendo os times a exportar
 * @return 1 se a exportacao foi bem sucedida, 0 em caso de erro
 */
static int bdtimes_exportar_csv(const BDTimes *bd) {
    // Nome do arquivo CSV de saida (sera criado no diretorio atual)
    const char *nome_arquivo = "bd_classificacao.csv";
    
    // Abre o arquivo em modo de escrita (sobrescreve se ja existir)
    FILE *f = fopen(nome_arquivo, "w");
    if (!f) {
        // Se falhar ao criar o arquivo, exibe mensagem de erro e retorna 0
        fprintf(stderr, "Erro ao criar arquivo CSV: %s\n", nome_arquivo);
        return 0;
    }

    // Define as larguras de cada coluna para alinhamento consistente
    // Estas larguras devem ser suficientes para acomodar os dados mais longos
    const int W_ID   = 3;   // Largura para o ID do time
    const int W_TIME = 12;  // Largura para o nome do time
    const int W_V    = 2;   // Largura para vitorias
    const int W_E    = 2;   // Largura para empates
    const int W_D    = 2;   // Largura para derrotas
    const int W_GM   = 3;   // Largura para gols marcados
    const int W_GS   = 3;   // Largura para gols sofridos
    const int W_S    = 3;   // Largura para saldo de gols
    const int W_PG   = 3;   // Largura para pontos ganhos

    // Escreve a linha de cabecalho com os nomes das colunas
    fprintf(f, "| %-*s | %-*s | %-*s | %-*s | %-*s | %-*s | %-*s | %-*s | %-*s |\n",
            W_ID, "ID", W_TIME, "Time", W_V, "V", W_E, "E", W_D, "D",
            W_GM, "GM", W_GS, "GS", W_S, "S", W_PG, "PG");

    // Escreve a linha separadora usando hifens para cada coluna
    fprintf(f, "|-");
    for (int i = 0; i < W_ID; i++) fputc('-', f);
    fprintf(f, "-|-");
    for (int i = 0; i < W_TIME; i++) fputc('-', f);
    fprintf(f, "-|-");
    for (int i = 0; i < W_V; i++) fputc('-', f);
    fprintf(f, "-|-");
    for (int i = 0; i < W_E; i++) fputc('-', f);
    fprintf(f, "-|-");
    for (int i = 0; i < W_D; i++) fputc('-', f);
    fprintf(f, "-|-");
    for (int i = 0; i < W_GM; i++) fputc('-', f);
    fprintf(f, "-|-");
    for (int i = 0; i < W_GS; i++) fputc('-', f);
    fprintf(f, "-|-");
    for (int i = 0; i < W_S; i++) fputc('-', f);
    fprintf(f, "-|-");
    for (int i = 0; i < W_PG; i++) fputc('-', f);
    fprintf(f, "-|\n");

    // Percorre todos os IDs possiveis de 0 a 9999
    // Esta abordagem garante que os times sejam exportados em ordem crescente de ID
    for (int id = 0; id < 10000; id++) {
        // Para cada ID, procura se existe um time correspondente
        for (int i = 0; i < bd->n; i++) {
            if (bd->times[i].id == id) {
                // Time encontrado! Vamos escrever seus dados no arquivo
                const Time *t = &bd->times[i];
                
                // Calcula os valores derivados (saldo e pontos)
                int saldo = time_saldo(t);     // Saldo = gols marcados - gols sofridos
                int pontos = time_pontos(t);   // Pontos = 3*vitorias + empates
                
                // Escreve uma linha formatada com todos os dados do time
                // Usa formatacao com largura fixa (%-*s para strings, %-*d para numeros)
                // O hifen (-) indica alinhamento a esquerda
                fprintf(f, "| %-*d | %-*s | %-*d | %-*d | %-*d | %-*d | %-*d | %-*d | %-*d |\n",
                        W_ID, t->id,           // ID do time
                        W_TIME, t->nome,       // Nome do time
                        W_V, t->v,             // Vitorias
                        W_E, t->e,             // Empates
                        W_D, t->d,             // Derrotas
                        W_GM, t->gm,           // Gols marcados
                        W_GS, t->gs,           // Gols sofridos
                        W_S, saldo,            // Saldo de gols
                        W_PG, pontos);         // Pontos ganhos
            }
        }
    }

    // Fecha o arquivo apos escrever todos os dados
    fclose(f);
    
    // Informa ao usuario que o arquivo foi criado com sucesso
    printf("[Sistema] Arquivo '%s' criado/atualizado com sucesso.\n", nome_arquivo);
    
    return 1;  // Retorna 1 indicando sucesso na operacao
}

/**
 * Imprime a tabela de classificacao dos times na tela em formato visual.
 * Alem de imprimir na tela, esta funcao tambem exporta os dados para um arquivo CSV.
 * 
 * A tabela e impressa com colunas alinhadas e separadas por pipes (|) para facilitar
 * a leitura. Os times sao ordenados por ID de forma crescente.
 * 
 * Colunas exibidas:
 * - ID: Identificador unico do time
 * - Time: Nome do time (truncado se muito longo)
 * - V: Numero de vitorias
 * - E: Numero de empates
 * - D: Numero de derrotas
 * - GM: Gols marcados pelo time
 * - GS: Gols sofridos pelo time
 * - S: Saldo de gols (GM - GS)
 * - PG: Pontos ganhos (3*V + E)
 * 
 * @param bd Ponteiro para a estrutura BDTimes contendo os dados dos times
 */
void bdtimes_imprimir_classificacao(const BDTimes *bd) {
    // Define as larguras visuais de cada coluna na tabela
    // Estas larguras determinam quantos caracteres/espacos cada coluna ocupara
    const int W_ID   = 3;   // Largura da coluna ID
    const int W_TIME = 12;  // Largura da coluna Time (nomes serao truncados se excederem)
    const int W_V    = 2;   // Largura da coluna Vitorias
    const int W_E    = 2;   // Largura da coluna Empates
    const int W_D    = 2;   // Largura da coluna Derrotas
    const int W_GM   = 3;   // Largura da coluna Gols Marcados
    const int W_GS   = 3;   // Largura da coluna Gols Sofridos
    const int W_S    = 3;   // Largura da coluna Saldo
    const int W_PG   = 3;   // Largura da coluna Pontos Ganhos

    // Imprime o cabecalho da tabela com os nomes das colunas
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

    // Imprime a linha separadora para alinhar visualmente a tabela
    // Esta linha contem hifens sob cada coluna
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

    // Percorre todos os IDs possiveis em ordem crescente (0 a 9999)
    // Esta estrategia garante que a saida seja ordenada por ID
    for (int id = 0; id < 10000; id++) {
        // Para cada ID, verifica se existe um time correspondente no banco de dados
        for (int i = 0; i < bd->n; i++) {
            if (bd->times[i].id == id) {
                // Time encontrado! Vamos imprimir seus dados
                const Time *t = &bd->times[i];
                char tmp[32];  // Buffer temporario para converter numeros em strings

                // Imprime o ID do time
                printf("| ");
                snprintf(tmp, sizeof(tmp), "%d", t->id);
                print_utf8_padded(tmp, W_ID);

                // Imprime o nome do time (sera truncado com '...' se exceder W_TIME)
                printf(" | ");
                print_utf8_padded(t->nome, W_TIME);

                // Imprime o numero de vitorias
                printf(" | ");
                snprintf(tmp, sizeof(tmp), "%d", t->v);
                print_utf8_padded(tmp, W_V);

                // Imprime o numero de empates
                printf(" | ");
                snprintf(tmp, sizeof(tmp), "%d", t->e);
                print_utf8_padded(tmp, W_E);

                // Imprime o numero de derrotas
                printf(" | ");
                snprintf(tmp, sizeof(tmp), "%d", t->d);
                print_utf8_padded(tmp, W_D);

                // Imprime os gols marcados
                printf(" | ");
                snprintf(tmp, sizeof(tmp), "%d", t->gm);
                print_utf8_padded(tmp, W_GM);

                // Imprime os gols sofridos
                printf(" | ");
                snprintf(tmp, sizeof(tmp), "%d", t->gs);
                print_utf8_padded(tmp, W_GS);

                // Imprime o saldo de gols (calculado pela funcao time_saldo)
                printf(" | ");
                snprintf(tmp, sizeof(tmp), "%d", time_saldo(t));
                print_utf8_padded(tmp, W_S);

                // Imprime os pontos ganhos (calculado pela funcao time_pontos)
                printf(" | ");
                snprintf(tmp, sizeof(tmp), "%d", time_pontos(t));
                print_utf8_padded(tmp, W_PG);

                printf(" |\n");
            }
        }
    }
    
    // Apos imprimir a tabela na tela, exporta os dados para arquivo CSV
    // Esta funcao cria/sobrescreve o arquivo "bd_classificacao.csv"
    bdtimes_exportar_csv(bd);
}

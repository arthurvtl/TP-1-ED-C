/**
 * Modulo: main.c
 * 
 * Ponto de entrada principal do Sistema de Gerenciamento de Partidas.
 * 
 * Este modulo implementa a interface de usuario (menu) e orquestra as
 * operacoes do sistema:
 * - Carregamento de dados (times e partidas) de arquivos CSV
 * - Menu interativo para consultas e visualizacoes
 * - Consulta de times por nome/prefixo
 * - Consulta de partidas por times participantes
 * - Exibicao e exportacao da tabela de classificacao
 * 
 * Fluxo de execucao:
 * 1. Carrega times do arquivo CSV
 * 2. Carrega partidas do arquivo CSV
 * 3. Aplica resultados das partidas nas estatisticas dos times
 * 4. Exibe menu interativo ate o usuario sair
 */

#include <stdio.h>
#include <string.h>
#include "bd_times.h"
#include "bd_partidas.h"
#include "utils.h"

// Inclui windows.h apenas se estiver compilando no Windows
#ifdef _WIN32
#include <windows.h>
#endif

/**
 * Exibe o menu principal do sistema.
 * 
 * Lista todas as opcoes disponiveis para o usuario:
 * - Consultar times por nome/prefixo
 * - Consultar partidas realizadas
 * - Imprimir tabela de classificacao completa
 * - Sair do sistema
 * 
 * Funcao auxiliar chamada em loop pelo main().
 */
static void menu(void) {
    printf("\nSistema de Gerenciamento de Partidas - Parte I\n");
    printf("1 - Consultar time\n");
    printf("2 - Consultar partidas\n");
    printf("6 - Imprimir tabela de classificacao\n");
    printf("Q - Sair\n");
    printf("Opcao: ");
}

/**
 * Implementa a funcionalidade de consulta de times.
 * 
 * Permite ao usuario buscar times digitando um nome completo ou prefixo.
 * A busca e case-insensitive e retorna todos os times que correspondem.
 * 
 * Para cada time encontrado, exibe:
 * - ID, Nome, Vitorias, Empates, Derrotas
 * - Gols Marcados, Gols Sofridos, Saldo, Pontos
 * 
 * Exemplos de busca:
 * - "Fla" encontra "Flamengo", "Fluminense"
 * - "Santos" encontra "Santos"
 * - "cor" encontra "Corinthians"
 * 
 * @param bdt Ponteiro para a base de dados de times
 */
static void consultar_time(BDTimes *bdt) {
    char buf[128];
    
    // Solicita o prefixo ao usuario
    printf("Digite o nome ou prefixo do time: ");
    if (!read_line(buf, sizeof(buf))) return;
    
    // Remove espacos em branco das pontas
    str_trim(buf);
    
    // Valida que o prefixo nao esta vazio
    if (buf[0] == '\0') {
        printf("Prefixo vazio.\n");
        return;
    }
    
    // Busca times que correspondem ao prefixo
    int indices[64];  // Array para armazenar indices dos times encontrados
    int total = bdtimes_buscar_por_prefixo(bdt, buf, indices, 64);
    
    // Verifica se algum time foi encontrado
    if (total <= 0) {
        printf("Nenhum time encontrado para prefixo: %s\n", buf);
        return;
    }
    
    // Imprime o cabecalho da tabela de resultados
    printf("\n| ID | Time | V | E | D | GM | GS | S | PG |\n");
    printf("|----|------|---|---|---|----|----|----|----|\n");
    
    // Imprime cada time encontrado
    for (int i = 0; i < total && i < 64; i++) {
        Time *t = &bdt->times[indices[i]];
        printf("| %d | %s | %d | %d | %d | %d | %d | %d | %d |\n",
            t->id, t->nome, t->v, t->e, t->d, t->gm, t->gs, 
            time_saldo(t), time_pontos(t));
    }
}

/**
 * Implementa a funcionalidade de consulta de partidas.
 * 
 * Submenu que permite buscar partidas de diferentes formas:
 * 1. Por time mandante (joga em casa)
 * 2. Por time visitante
 * 3. Por qualquer time (mandante ou visitante)
 * 
 * Para cada filtro, o usuario digita um nome ou prefixo do time
 * e o sistema lista todas as partidas correspondentes com placares.
 * 
 * O usuario pode realizar multiplas consultas antes de retornar
 * ao menu principal.
 * 
 * @param bdp Ponteiro para a base de dados de partidas
 * @param bdt Ponteiro para a base de dados de times (para buscar nomes)
 */
static void consultar_partidas(const BDPartidas *bdp, const BDTimes *bdt) {
    // Loop infinito - usuario sai explicitamente escolhendo opcao 4
    for (;;) {
        // Exibe as opcoes de filtragem
        printf("\nEscolha o modo de consulta:\n");
        printf("1 - Por time mandante\n");
        printf("2 - Por time visitante\n");
        printf("3 - Por time mandante ou visitante\n");
        printf("4 - Retornar ao menu principal\n");
        printf("Opcao: ");
        
        // Le a opcao escolhida
        char op[8];
        if (!read_line(op, sizeof(op))) return;
        
        // Opcao 4: retorna ao menu principal
        if (op[0] == '4') return;

        // Solicita o prefixo do time a buscar
        char prefixo[128];
        printf("Digite o nome: ");
        if (!read_line(prefixo, sizeof(prefixo))) return;
        
        // Remove espacos em branco
        str_trim(prefixo);
        
        // Valida que o prefixo nao esta vazio
        if (prefixo[0] == '\0') {
            printf("Prefixo vazio.\n");
            continue;  // Volta ao inicio do loop para nova tentativa
        }

        // Executa a funcao de listagem apropriada conforme a opcao
        if (op[0] == '1') {
            // Lista partidas onde o time e mandante
            bdpartidas_listar_por_mandante_prefixo(bdp, bdt, prefixo);
        } else if (op[0] == '2') {
            // Lista partidas onde o time e visitante
            bdpartidas_listar_por_visitante_prefixo(bdp, bdt, prefixo);
        } else if (op[0] == '3') {
            // Lista partidas onde o time e mandante OU visitante
            bdpartidas_listar_por_qualquer_prefixo(bdp, bdt, prefixo);
        } else {
            // Opcao invalida
            printf("Opcao invalida.\n");
        }
    }
}

/**
 * Funcao principal do programa.
 * 
 * Responsabilidades:
 * 1. Configura a console para aceitar caracteres UTF-8
 * 2. Determina os caminhos dos arquivos CSV (via argumentos ou padroes)
 * 3. Carrega dados de times e partidas
 * 4. Calcula estatisticas aplicando resultados das partidas
 * 5. Executa loop principal do menu interativo
 * 6. Processa escolhas do usuario ate sair
 * 
 * Argumentos de linha de comando (opcionais):
 * - argv[1]: Caminho do arquivo CSV de times
 * - argv[2]: Caminho do arquivo CSV de partidas
 * 
 * Se nao fornecidos, usa "times.csv" e "partidas.csv" do diretorio atual.
 * 
 * @param argc Numero de argumentos da linha de comando
 * @param argv Array de strings com os argumentos
 * @return 0 se execucao normal, 1 se houver erro critico
 */
int main(int argc, char *argv[]) {
    // Configura a console do Windows para aceitar UTF-8
    // Necessario para exibir corretamente nomes com acentuacao
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif

    // Define os caminhos padrao dos arquivos CSV
    const char *times_path = "times.csv";
    const char *partidas_path = "partidas.csv";
    
    // Verifica se o usuario passou caminhos customizados via linha de comando
    if (argc >= 3) {
        // Usa os caminhos fornecidos pelo usuario
        times_path = argv[1];
        partidas_path = argv[2];
    } else {
        // Informa ao usuario como usar argumentos customizados
        printf("Dica: voce pode passar caminhos dos CSVs: %s <times.csv> <partidas.csv>\n", argv[0]);
        printf("Tentando abrir 'times.csv' e 'partidas.csv' do diretorio atual.\n");
    }

    // Declara e inicializa as estruturas de dados principais
    BDTimes bdt;        // Base de dados de times
    BDPartidas bdp;     // Base de dados de partidas
    bdtimes_init(&bdt);
    bdpartidas_init(&bdp);

    // Carrega os times do arquivo CSV
    if (!bdtimes_carregar_csv(&bdt, times_path)) {
        // Erro critico: sem times, o sistema nao pode funcionar
        fprintf(stderr, "Falha ao carregar times.\n");
        return 1;  // Encerra com codigo de erro
    }
    
    // Carrega as partidas do arquivo CSV
    // Nota: As estatisticas dos times comecam zeradas e serao
    // calculadas na proxima etapa ao aplicar as partidas
    if (!bdpartidas_carregar_csv(&bdp, partidas_path)) {
        // Erro ao carregar partidas, mas o sistema pode continuar
        // funcionando com consulta de times (estatisticas zeradas)
        fprintf(stderr, "Falha ao carregar partidas.\n");
    }
    
    // Aplica os resultados de todas as partidas nas estatisticas dos times
    // Esta funcao atualiza: vitorias, empates, derrotas, gols marcados/sofridos
    bdpartidas_aplicar_em_bdtimes(&bdp, &bdt);

    // Loop principal do programa - executa ate o usuario escolher sair
    for (;;) {
        // Exibe o menu e aguarda escolha do usuario
        menu();
        
        char op[8];
        if (!read_line(op, sizeof(op))) break;  // EOF ou erro de leitura
        
        // Verifica se o usuario quer sair (Q ou q)
        if (op[0] == 'Q' || op[0] == 'q') break;
        
        // Processa a opcao escolhida
        switch (op[0]) {
            case '1':
                // Opcao 1: Consultar time por nome/prefixo
                consultar_time(&bdt);
                break;
                
            case '2':
                // Opcao 2: Consultar partidas (submenu)
                consultar_partidas(&bdp, &bdt);
                break;
                
            case '6':
                // Opcao 6: Imprimir e exportar tabela de classificacao
                printf("Imprimindo classificacao.\n");
                bdtimes_imprimir_classificacao(&bdt);
                break;
                
            default:
                // Opcao nao reconhecida
                printf("Opcao invalida.\n");
                break;
        }
    }

    // Mensagem de encerramento
    printf("Encerrando.\n");
    return 0;  // Execucao bem sucedida
}

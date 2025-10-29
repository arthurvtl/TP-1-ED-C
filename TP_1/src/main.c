#include <stdio.h>
#include <string.h>
#include "bd_times.h"
#include "bd_partidas.h"
#include "utils.h"

static void menu() {
    printf("\nSistema de Gerenciamento de Partidas - Parte I\n");
    printf("1 - Consultar time\n");
    printf("2 - Consultar partidas\n");
    printf("6 - Imprimir tabela de classificacao\n");
    printf("Q - Sair\n");
    printf("Opcao: ");
}

static void consultar_time(BDTimes *bdt) {
    char buf[128];
    printf("Digite o nome ou prefixo do time: ");
    if (!read_line(buf, sizeof(buf))) return;
    str_trim(buf);
    if (buf[0] == '\0') {
        printf("Prefixo vazio.\n");
        return;
    }
    int indices[64];
    int total = bdtimes_buscar_por_prefixo(bdt, buf, indices, 64);
    if (total <= 0) {
        printf("Nenhum time encontrado para prefixo: %s\n", buf);
        return;
    }
    printf("\n| ID | Time | V | E | D | GM | GS | S | PG |\n");
    printf("|----|------|---|---|---|----|----|----|----|\n");
    for (int i = 0; i < total && i < 64; i++) {
        Time *t = &bdt->times[indices[i]];
        printf("| %d | %s | %d | %d | %d | %d | %d | %d | %d |\n",
            t->id, t->nome, t->v, t->e, t->d, t->gm, t->gs, time_saldo(t), time_pontos(t));
    }
}

static void consultar_partidas(const BDPartidas *bdp, const BDTimes *bdt) {
    for (;;) {
        printf("\nEscolha o modo de consulta:\n");
        printf("1 - Por time mandante\n");
        printf("2 - Por time visitante\n");
        printf("3 - Por time mandante ou visitante\n");
        printf("4 - Retornar ao menu principal\n");
        printf("Opcao: ");
        char op[8];
        if (!read_line(op, sizeof(op))) return;
        if (op[0] == '4') return;

        char prefixo[128];
        printf("Digite o nome: ");
        if (!read_line(prefixo, sizeof(prefixo))) return;
        str_trim(prefixo);
        if (prefixo[0] == '\0') {
            printf("Prefixo vazio.\n");
            continue;
        }

        if (op[0] == '1') {
            bdpartidas_listar_por_mandante_prefixo(bdp, bdt, prefixo);
        } else if (op[0] == '2') {
            bdpartidas_listar_por_visitante_prefixo(bdp, bdt, prefixo);
        } else if (op[0] == '3') {
            bdpartidas_listar_por_qualquer_prefixo(bdp, bdt, prefixo);
        } else {
            printf("Opcao invalida.\n");
        }
    }
}

int main(int argc, char *argv[]) {
    const char *times_path = "times.csv";
    const char *partidas_path = "partidas.csv";
    if (argc >= 3) {
        times_path = argv[1];
        partidas_path = argv[2];
    } else {
        printf("Dica: voce pode passar caminhos dos CSVs: %s <times.csv> <partidas.csv>\n", argv[0]);
        printf("Tentando abrir 'times.csv' e 'partidas.csv' do diretorio atual.\n");
    }

    BDTimes bdt;
    BDPartidas bdp;
    bdtimes_init(&bdt);
    bdpartidas_init(&bdp);

    if (!bdtimes_carregar_csv(&bdt, times_path)) {
        fprintf(stderr, "Falha ao carregar times.\n");
        return 1;
    }
    // Stats comecam zeradas; so depois de carregar partidas acumulamos.
    if (!bdpartidas_carregar_csv(&bdp, partidas_path)) {
        fprintf(stderr, "Falha ao carregar partidas.\n");
        // Ainda assim podemos consultar time, mas stats ficarão zeradas.
    }
    bdpartidas_aplicar_em_bdtimes(&bdp, &bdt);

    for (;;) {
        menu();
        char op[8];
        if (!read_line(op, sizeof(op))) break;
        if (op[0] == 'Q' || op[0] == 'q') break;
        switch (op[0]) {
            case '1': consultar_time(&bdt); break;
            case '2': consultar_partidas(&bdp, &bdt); break;
            case '6': 
                printf("Imprimindo classificacao.\n");
                bdtimes_imprimir_classificacao(&bdt); 
                break;
            default: printf("Opcao invalida.\n"); break;
        }
    }

    printf("Encerrando.\n");
    return 0;
}
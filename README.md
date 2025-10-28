### README — Campeonato Computacional de Futebol (Parte I) em C

#### Visão Geral
Este projeto implementa, em C, um sistema simples de gerenciamento de campeonato de futebol para consulta de times, partidas e impressão de classificação, conforme o enunciado da Parte I. Os dados são lidos de arquivos CSV (times e partidas). O programa funciona em Windows e Linux, lê nomes com acentos (UTF‑8) e imprime tabelas alinhadas visualmente.

#### Funcionalidades
- Carregar `times.csv` e um arquivo de partidas escolhido pelo usuário:
  - `data/partidas/partidas_vazio.csv`
  - `data/partidas/partidas_parcial.csv`
  - `data/partidas/partidas_completo.csv`
- Menu interativo:
  1) Consultar time por prefixo (case-insensitive, com acentos)
  2) Consultar partidas por mandante
  3) Consultar partidas por visitante
  4) Consultar partidas por mandante ou visitante
  6) Imprimir tabela de classificação (ordem por ID, Parte I)
  Q) Sair
- Impressão da classificação com colunas alinhadas e nomes UTF‑8.

#### Estrutura do Projeto
- include/
  - bd_times.h, bd_partidas.h, utils.h
- src/
  - main.c, bd_times.c, bd_partidas.c, utils.c
- data/
  - times.csv
  - partidas/
    - partidas_vazio.csv
    - partidas_parcial.csv
    - partidas_completo.csv
- bin/ (gerado pelo build)
- build/ (gerado pelo build)
- Makefile

#### Requisitos
- Compilador C (GCC recomendado).
- Windows ou Linux.
- Terminal configurado para UTF‑8 para exibir acentos:
  - Windows PowerShell: execute `chcp 65001` antes de rodar o programa.

#### Como Compilar
- No diretório raiz do projeto:
make

- O binário será gerado em:
  - Windows: `bin\tp_parte1.exe` (ou `bin\tp_parte1`)
  - Linux: `bin/tp_parte1`

- Para recompilar do zero:
make clean
make


#### Como Executar
Passe sempre dois argumentos: caminho de `times.csv` e o arquivo de partidas desejado.

- Cenário vazio:
make run ARGS="data/times.csv data/partidas/partidas_vazio.csv"

- Cenário parcial:
make run ARGS="data/times.csv data/partidas/partidas_parcial.csv"

- Cenário completo:
make run ARGS="data/times.csv data/partidas/partidas_completo.csv"


- Executando diretamente o binário:
  - Windows:
    ```
    .\bin\tp_parte1 data\times.csv data\partidas\partidas_completo.csv
    ```
  - Linux:
    ```
    ./bin/tp_parte1 data/times.csv data/partidas/partidas_completo.csv
    ```

Dica (Windows): se os acentos aparecerem errados, rode `chcp 65001` no PowerShell antes do comando.

#### Formato dos CSVs
- times.csv
  - Cabeçalho: `ID,Time`
  - Exemplo:
    ```
    ID,Time
    0,JAVAlis
    1,ESCorpiões
    2,SemCTRL
    ...
    ```

- partidas_*.csv
  - Cabeçalho: `ID,Time1ID,Time2ID,GolsTime1,GolsTime2`
  - Exemplo:
    ```
    ID,Time1ID,Time2ID,GolsTime1,GolsTime2
    0,5,7,4,1
    1,9,1,2,5
    ...
    ```

Observações:
- Arquivos devem estar salvos em UTF‑8. No VS Code: “Save with Encoding -> UTF‑8”.
- Linhas malformadas são ignoradas com aviso no stderr.

#### Notas de Implementação
- TADs:
  - Time e BDTimes: carrega times, busca por ID/prefixo, acumula estatísticas, imprime classificação.
  - Partida e BDPartidas: carrega partidas, aplica resultados em BDTimes, consultas por prefixo.
- Parsing robusto para CRLF (Windows) e LF (Linux).
- Alinhamento de colunas em UTF‑8:
  - Cálculo de “largura visual” por code points UTF‑8.
  - Truncamento com “…” quando ultrapassar a largura da coluna.
- Bibliotecas padrão apenas: stdio.h, stdlib.h, string.h, ctype.h.

#### Atalhos Úteis
- Compilar e executar com dataset completo:
  - Windows:
    ```
    make clean && make && .\bin\tp_parte1 data\times.csv data\partidas\partidas_completo.csv
    ```
  - Linux:
    ```
    make clean && make && ./bin/tp_parte1 data/times.csv data/partidas/partidas_completo.csv
    ```

#### Solução de Problemas
- Erro “No rule to make target 'run'”:
  - Use `make` e rode o binário diretamente, ou garanta que o Makefile inclui o alvo `run` (já incluído neste projeto).
- Erro ao criar diretórios no Windows (“mkdir -p”):
  - O Makefile já detecta o SO e usa o comando correto.
- Acentos ficam incorretos no Windows:
  - Execute `chcp 65001` no PowerShell e use uma fonte com suporte UTF‑8.
- Linhas de partidas ignoradas:
  - Verifique se o CSV possui cabeçalho correto e está em UTF‑8; o parser já trata CRLF.

#### Limitações (Parte I)
- Classificação em ordem de ID (sem ordenação por pontos).
- Sem persistência além do carregamento dos CSVs.

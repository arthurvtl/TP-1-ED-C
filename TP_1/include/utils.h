/**
 * Header: utils.h
 * 
 * Define funcoes utilitarias de uso geral para manipulacao de strings,
 * entrada/saida e processamento de texto UTF-8.
 * 
 * Este modulo oferece funcionalidades para:
 * - Manipulacao de strings (trim, lowercase, prefixos)
 * - Leitura segura de entrada do usuario
 * - Conversao segura de strings para inteiros
 * - Tratamento correto de texto UTF-8 (contagem de caracteres, alinhamento)
 * 
 * As funcoes UTF-8 sao essenciais para exibir nomes de times que podem
 * conter caracteres acentuados ou especiais, garantindo alinhamento
 * correto nas tabelas mesmo com caracteres multi-byte.
 */

#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdbool.h>

// ========== Funcoes de manipulacao de strings ==========

/**
 * Remove espacos em branco no inicio e fim de uma string.
 * 
 * Modifica a string in-place, removendo espacos, tabs, quebras de linha
 * e outros caracteres de espacamento (isspace) do inicio e fim.
 * O conteudo interno da string e preservado.
 * 
 * Exemplo: "  Flamengo  " -> "Flamengo"
 * 
 * @param s String a ser processada (modificada in-place)
 */
void str_trim(char *s);

/**
 * Converte todos os caracteres ASCII para minusculas.
 * 
 * Modifica a string in-place, convertendo apenas caracteres ASCII
 * (A-Z) para suas versoes minusculas (a-z).
 * Caracteres nao-ASCII (como acentos) nao sao afetados.
 * 
 * Exemplo: "FLAMENGO" -> "flamengo"
 * 
 * @param s String a ser convertida (modificada in-place)
 */
void str_to_lower(char *s);

/**
 * Verifica se uma string comeca com um prefixo especifico (case-insensitive).
 * 
 * Compara o inicio de 'text' com 'prefix', ignorando diferencas entre
 * maiusculas e minusculas (apenas para caracteres ASCII).
 * 
 * Exemplos:
 * - str_starts_with_case_insensitive("Flamengo", "Fla") -> true
 * - str_starts_with_case_insensitive("flamengo", "FLA") -> true
 * - str_starts_with_case_insensitive("Santos", "Fla") -> false
 * 
 * @param text String onde buscar o prefixo
 * @param prefix Prefixo a ser procurado no inicio de text
 * @return true se text comeca com prefix (ignorando caixa), false caso contrario
 */
bool str_starts_with_case_insensitive(const char *text, const char *prefix);

// ========== Funcoes de entrada/saida ==========

/**
 * Le uma linha completa da entrada padrao (stdin).
 * 
 * Le ate encontrar '\n' ou preencher o buffer.
 * O caractere '\n' final e removido automaticamente.
 * A string retornada sempre termina com '\0'.
 * 
 * @param buf Buffer onde a linha sera armazenada
 * @param size Tamanho do buffer (incluindo espaco para '\0')
 * @return 1 se a leitura foi bem sucedida, 0 em caso de erro ou EOF
 */
int read_line(char *buf, int size);

/**
 * Pausa a execucao aguardando o usuario pressionar ENTER.
 * 
 * Exibe uma mensagem e consome todos os caracteres ate encontrar '\n'.
 * Util para pausar o programa antes de limpar a tela ou continuar.
 */
void pause_prompt(void);

/**
 * Converte uma string para inteiro de forma segura.
 * 
 * Diferente de atoi(), esta funcao:
 * - Valida que a string contem apenas digitos (e sinal opcional)
 * - Detecta overflow (valores maiores que INT_MAX)
 * - Retorna status de sucesso/falha
 * 
 * Exemplos validos: "123", "-45", "0"
 * Exemplos invalidos: "12a", "abc", "999999999999999"
 * 
 * @param s String a ser convertida
 * @param out Ponteiro onde o valor convertido sera armazenado (apenas se sucesso)
 * @return 1 se a conversao foi bem sucedida, 0 se houver erro
 */
int safe_atoi(const char *s, int *out);

// ========== Funcoes para manipulacao de UTF-8 ==========

/**
 * Calcula a largura visual de uma string UTF-8.
 * 
 * Retorna o numero de code points (caracteres) na string, contando
 * corretamente caracteres multi-byte UTF-8.
 * 
 * IMPORTANTE: Esta funcao conta code points, nao bytes. Por exemplo:
 * - "abc" tem 3 code points (3 bytes)
 * - "ção" tem 4 code points (5 bytes, pois 'ç' e 'ã' usam 2 bytes cada)
 * 
 * Limitacoes: Nao lida com caracteres combinados (combining characters)
 * 
 * @param s String UTF-8 a ser medida
 * @return Numero de code points (largura visual) da string
 */
int utf8_len(const char *s);

/**
 * Imprime uma string UTF-8 ajustada para uma largura fixa.
 * 
 * Esta funcao garante que a saida ocupe exatamente 'width' code points:
 * - Se a string e mais curta: preenche com espacos a direita
 * - Se a string e mais longa: trunca e adiciona '...' (ellipsis)
 * - Se a string tem exatamente width: imprime como esta
 * 
 * Essencial para criar tabelas alinhadas com texto UTF-8.
 * 
 * Exemplos (width=10):
 * - "Fla" -> "Fla       " (7 espacos adicionados)
 * - "Flamengo" -> "Flamengo  " (2 espacos adicionados)
 * - "Independiente" -> "Independen..." (truncado com ellipsis)
 * 
 * @param s String UTF-8 a ser impressa
 * @param width Largura visual desejada (em code points)
 */
void print_utf8_padded(const char *s, int width);

#endif

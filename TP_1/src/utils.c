/**
 * Modulo: utils.c
 * 
 * Implementa funcoes utilitarias de uso geral para manipulacao de strings,
 * entrada/saida e processamento de texto UTF-8.
 * 
 * Este modulo fornece ferramentas essenciais usadas por todo o sistema:
 * - Limpeza e normalizacao de strings (trim, lowercase)
 * - Comparacoes case-insensitive para buscas flexiveis
 * - Leitura segura de entrada do usuario
 * - Conversao robusta de strings para numeros
 * - Manipulacao correta de texto UTF-8 para alinhamento de tabelas
 * 
 * As funcoes UTF-8 garantem que nomes de times com acentuacao sejam
 * exibidos corretamente e alinhados nas tabelas de forma visual.
 */

#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/**
 * Remove espacos em branco do inicio e fim de uma string.
 * 
 * Algoritmo:
 * 1. Encontra o primeiro caractere nao-espaco do inicio
 * 2. Encontra o ultimo caractere nao-espaco do fim
 * 3. Copia apenas a parte central (sem espacos nas pontas)
 * 4. Adiciona terminador nulo ao final
 * 
 * A string e modificada in-place, sem alocar memoria adicional.
 * Espacos internos sao preservados.
 * 
 * @param s String a ser processada (modificada in-place)
 */
void str_trim(char *s) {
    // Verifica se o ponteiro e valido
    if (!s) return;
    
    int len = (int)strlen(s);
    
    // Encontra o indice do primeiro caractere nao-espaco
    int i = 0;
    while (i < len && isspace((unsigned char)s[i])) {
        i++;
    }
    
    // Encontra o indice do ultimo caractere nao-espaco
    int j = len - 1;
    while (j >= i && isspace((unsigned char)s[j])) {
        j--;
    }
    
    // Copia a substring sem espacos para o inicio do buffer
    int k = 0;
    while (i <= j) {
        s[k++] = s[i++];
    }
    
    // Adiciona terminador nulo
    s[k] = '\0';
}

/**
 * Converte todos os caracteres ASCII para minusculas.
 * 
 * Percorre a string caractere por caractere convertendo
 * letras maiusculas (A-Z) para minusculas (a-z).
 * Outros caracteres (incluindo acentuados) nao sao modificados.
 * 
 * A string e modificada in-place.
 * 
 * @param s String a ser convertida (modificada in-place)
 */
void str_to_lower(char *s) {
    // Verifica se o ponteiro e valido
    if (!s) return;
    
    // Percorre cada caractere ate encontrar o terminador nulo
    for (; *s; ++s) {
        // Converte para minuscula (tolower) e armazena de volta
        *s = (char)tolower((unsigned char)*s);
    }
}

/**
 * Implementacao interna da comparacao case-insensitive.
 * 
 * Esta funcao auxiliar realiza a comparacao real sem validar
 * se os ponteiros sao nulos (isso e feito pela funcao publica).
 * 
 * Algoritmo:
 * 1. Compara caractere por caractere
 * 2. Converte ambos para minuscula antes de comparar
 * 3. Para se encontrar diferenca ou fim do prefixo
 * 
 * @param text String onde buscar o prefixo
 * @param prefix Prefixo a ser procurado
 * @return 1 se text comeca com prefix, 0 caso contrario
 */
static int starts_with_case_insensitive_impl(const char *text, const char *prefix) {
    // Percorre o prefixo ate o final
    while (*prefix) {
        // Se text terminou antes do prefixo, nao e um match
        if (*text == '\0') return 0;
        
        // Converte ambos os caracteres para minuscula
        char a = (char)tolower((unsigned char)*text);
        char b = (char)tolower((unsigned char)*prefix);
        
        // Se sao diferentes, nao e um match
        if (a != b) return 0;
        
        // Avanca para o proximo caractere
        text++;
        prefix++;
    }
    
    // Chegou ao fim do prefixo sem encontrar diferencas
    return 1;
}

/**
 * Verifica se uma string comeca com um prefixo (case-insensitive).
 * 
 * Funcao publica que valida os parametros e chama a implementacao interna.
 * A comparacao ignora diferencas entre maiusculas e minusculas.
 * 
 * Exemplos:
 * - str_starts_with_case_insensitive("Flamengo", "fla") retorna true
 * - str_starts_with_case_insensitive("PALMEIRAS", "pal") retorna true
 * - str_starts_with_case_insensitive("Santos", "cor") retorna false
 * 
 * @param text String onde buscar o prefixo
 * @param prefix Prefixo a ser procurado no inicio de text
 * @return true se text comeca com prefix (ignorando caixa), false caso contrario
 */
bool str_starts_with_case_insensitive(const char *text, const char *prefix) {
    // Valida os parametros (nao podem ser NULL)
    if (!text || !prefix) return false;
    
    // Chama a implementacao interna e converte o resultado para bool
    return starts_with_case_insensitive_impl(text, prefix) ? true : false;
}

/**
 * Le uma linha completa da entrada padrao (stdin).
 * 
 * Usa fgets() internamente, mas remove o '\n' final automaticamente,
 * tornando mais conveniente o processamento da entrada.
 * 
 * Comportamento:
 * - Le ate encontrar '\n' ou preencher o buffer
 * - Remove o '\n' final se presente
 * - Garante que a string termina com '\0'
 * 
 * @param buf Buffer onde a linha sera armazenada
 * @param size Tamanho do buffer (incluindo espaco para '\0')
 * @return 1 se a leitura foi bem sucedida, 0 em caso de erro ou EOF
 */
int read_line(char *buf, int size) {
    // Tenta ler uma linha com fgets
    if (!fgets(buf, size, stdin)) {
        // Falha ao ler (EOF ou erro)
        return 0;
    }
    
    // Remove o '\n' final se presente
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }
    
    return 1;  // Leitura bem sucedida
}

/**
 * Pausa a execucao aguardando o usuario pressionar ENTER.
 * 
 * Exibe uma mensagem e consome todos os caracteres do buffer de entrada
 * ate encontrar '\n'. Util para pausar o programa e permitir que o
 * usuario leia mensagens antes de continuar.
 * 
 * Nota: Se houver caracteres no buffer de entrada antes da chamada,
 * eles serao consumidos ate encontrar um '\n'.
 */
void pause_prompt(void) {
    // Exibe mensagem ao usuario
    printf("Pressione ENTER para continuar...");
    
    // Consome todos os caracteres ate encontrar newline ou EOF
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // Loop vazio - apenas consome caracteres
    }
}

/**
 * Converte uma string para inteiro de forma segura.
 * 
 * Diferente de atoi() da biblioteca padrao, esta funcao:
 * 1. Valida que todos os caracteres sao digitos (exceto sinal inicial)
 * 2. Detecta overflow antes de acontecer
 * 3. Retorna status de sucesso/falha
 * 4. Suporta numeros negativos (sinal '-' no inicio)
 * 
 * Algoritmo:
 * 1. Verifica se ha sinal negativo no inicio
 * 2. Valida que todos os caracteres restantes sao digitos
 * 3. Converte digito por digito, verificando overflow a cada passo
 * 4. Aplica o sinal no final
 * 
 * Exemplos validos: "123", "-456", "0"
 * Exemplos invalidos: "12a", "abc", "", "999999999999" (overflow)
 * 
 * @param s String a ser convertida
 * @param out Ponteiro onde o valor convertido sera armazenado (apenas se sucesso)
 * @return 1 se a conversao foi bem sucedida, 0 se houver erro
 */
int safe_atoi(const char *s, int *out) {
    // Valida que a string nao e NULL nem vazia
    if (!s || !*s) return 0;
    
    // Verifica se ha sinal negativo no inicio
    int sign = 1;
    const char *p = s;
    if (*p == '-') {
        sign = -1;
        p++;  // Pula o sinal
    }
    
    // Deve haver pelo menos um digito apos o sinal (se houver)
    if (!isdigit((unsigned char)*p)) return 0;
    
    // Converte digito por digito
    int val = 0;
    while (*p) {
        // Verifica se o caractere atual e um digito
        if (!isdigit((unsigned char)*p)) return 0;
        
        // Extrai o valor do digito ('0' = 48, '1' = 49, etc.)
        int d = *p - '0';
        
        // Verifica overflow ANTES de fazer a multiplicacao/adicao
        // INT_MAX = 2147483647
        // Se val > (INT_MAX - d) / 10, entao val*10+d causaria overflow
        if (val > (2147483647 - d) / 10) return 0;
        
        // Acumula o digito no resultado
        val = val * 10 + d;
        p++;
    }
    
    // Aplica o sinal e armazena no parametro de saida
    *out = val * sign;
    return 1;  // Sucesso
}

// ========== Funcoes auxiliares de UTF-8 para alinhamento ==========

/**
 * Retorna a quantidade de bytes de um code point UTF-8.
 * 
 * Determina quantos bytes compõem o proximo caractere UTF-8 baseado
 * no primeiro byte (prefixo):
 * - 0xxxxxxx: 1 byte (ASCII: 0x00-0x7F)
 * - 110xxxxx: 2 bytes (0xC0-0xDF)
 * - 1110xxxx: 3 bytes (0xE0-0xEF)
 * - 11110xxx: 4 bytes (0xF0-0xF7)
 * 
 * Se o byte for invalido, trata como caractere de 1 byte.
 * 
 * @param p Ponteiro para o primeiro byte do code point
 * @return Numero de bytes do code point (1, 2, 3 ou 4)
 */
static int utf8_cp_bytes(const unsigned char *p) {
    // Caractere ASCII de 1 byte (bit mais significativo = 0)
    if (*p < 0x80) return 1;
    
    // Code point de 2 bytes (comeca com 110)
    if ((*p & 0xE0) == 0xC0) return 2;
    
    // Code point de 3 bytes (comeca com 1110)
    if ((*p & 0xF0) == 0xE0) return 3;
    
    // Code point de 4 bytes (comeca com 11110)
    if ((*p & 0xF8) == 0xF0) return 4;
    
    // Byte invalido: trata como caractere isolado de 1 byte
    return 1;
}

/**
 * Calcula a largura visual de uma string UTF-8.
 * 
 * Conta o numero de code points (caracteres) na string, tratando
 * corretamente sequencias multi-byte UTF-8.
 * 
 * Exemplos:
 * - "abc" -> 3 code points (3 bytes total)
 * - "São Paulo" -> 9 code points (10 bytes: 'ã' usa 2 bytes)
 * - "日本" -> 2 code points (6 bytes: cada kanji usa 3 bytes)
 * 
 * IMPORTANTE: Conta code points, nao bytes. Um caractere pode
 * ocupar multiplos bytes em UTF-8.
 * 
 * Limitacao: Nao lida com caracteres combinados (combining characters)
 * ou modificadores de emoji.
 * 
 * @param s String UTF-8 a ser medida
 * @return Numero de code points (largura visual) da string
 */
int utf8_len(const char *s) {
    // Valida o ponteiro
    if (!s) return 0;
    
    int count = 0;  // Contador de code points
    const unsigned char *p = (const unsigned char*)s;
    
    // Percorre a string ate o terminador nulo
    while (*p) {
        // Descobre quantos bytes tem o proximo code point
        int n = utf8_cp_bytes(p);
        
        // Pula todos os bytes deste code point
        p += n;
        
        // Incrementa o contador de caracteres
        count++;
    }
    
    return count;
}

/**
 * Copia um numero especifico de code points UTF-8.
 * 
 * Funcao auxiliar que copia ate 'width' code points da string 's'
 * para o buffer 'dst', respeitando o limite de 'cap' bytes.
 * 
 * Diferente de strncpy, esta funcao:
 * - Copia code points completos (nao quebra no meio de um caractere multi-byte)
 * - Para ao atingir 'width' caracteres OU 'cap' bytes
 * - Adiciona terminador nulo se houver espaco
 * 
 * @param dst Buffer de destino
 * @param cap Capacidade do buffer em bytes
 * @param s String UTF-8 de origem
 * @param width Numero maximo de code points a copiar
 * @return Numero de bytes escritos no buffer (sem contar '\0')
 */
static int utf8_copy_n_cps(char *dst, int cap, const char *s, int width) {
    int bytes = 0;  // Bytes escritos ate agora
    int cps = 0;    // Code points copiados ate agora
    const unsigned char *p = (const unsigned char*)s;
    
    // Copia ate atingir width code points ou fim da string
    while (*p && cps < width) {
        // Descobre quantos bytes tem o proximo code point
        int n = utf8_cp_bytes(p);
        
        // Verifica se ha espaco no buffer para este code point completo
        if (bytes + n >= cap) break;
        
        // Copia todos os bytes deste code point
        for (int i = 0; i < n; i++) {
            dst[bytes++] = (char)p[i];
        }
        
        p += n;
        cps++;
    }
    
    // Adiciona terminador nulo se houver espaco
    if (bytes < cap) {
        dst[bytes] = '\0';
    }
    
    return bytes;
}

/**
 * Imprime uma string UTF-8 ajustada para uma largura fixa.
 * 
 * Garante que a saida ocupe exatamente 'width' code points (caracteres):
 * - Se a string e mais curta: preenche com espacos a direita
 * - Se a string e mais longa: trunca e adiciona '…' (ellipsis U+2026)
 * - Se a string tem exatamente width: imprime como esta
 * 
 * Esta funcao e essencial para criar tabelas alinhadas com texto UTF-8,
 * pois garante que cada coluna tenha largura visual consistente.
 * 
 * Exemplos (width=10):
 * - "Fla"           -> "Fla       " (7 espacos adicionados)
 * - "Flamengo"      -> "Flamengo  " (2 espacos adicionados)
 * - "São Paulo"     -> "São Paulo " (1 espaco adicionado)
 * - "Internacional" -> "Internacio…" (truncado com ellipsis)
 * 
 * Nota: O ellipsis '…' (U+2026) ocupa 1 code point mas usa 3 bytes em UTF-8.
 * 
 * @param s String UTF-8 a ser impressa
 * @param width Largura visual desejada (em code points)
 */
void print_utf8_padded(const char *s, int width) {
    // Trata NULL como string vazia
    if (!s) s = "";
    
    // Calcula a largura visual atual da string
    int vis = utf8_len(s);
    
    // Caso 1: String tem exatamente a largura desejada
    if (vis == width) {
        fputs(s, stdout);
        return;
    }
    
    // Caso 2: String e mais curta - preenche com espacos
    if (vis < width) {
        fputs(s, stdout);
        for (int i = 0; i < width - vis; i++) {
            putchar(' ');
        }
        return;
    }
    
    // Caso 3: String e mais longa - trunca e adiciona ellipsis
    // Reserva 1 caractere para o ellipsis
    int keep = width - 1;
    if (keep < 0) keep = 0;
    
    // Copia apenas os primeiros 'keep' code points
    char buf[256];
    utf8_copy_n_cps(buf, sizeof(buf), s, keep);
    fputs(buf, stdout);
    
    // Adiciona o caractere ellipsis '…' (U+2026)
    // Em UTF-8: 0xE2 0x80 0xA6
    putchar((char)0xE2);
    putchar((char)0x80);
    putchar((char)0xA6);
}

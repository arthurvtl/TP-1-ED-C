#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void str_trim(char *s) {
    if (!s) return;
    int len = (int)strlen(s);
    int i = 0;
    while (i < len && isspace((unsigned char)s[i])) i++;
    int j = len - 1;
    while (j >= i && isspace((unsigned char)s[j])) j--;
    int k = 0;
    while (i <= j) s[k++] = s[i++];
    s[k] = '\0';
}

void str_to_lower(char *s) {
    if (!s) return;
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

static int starts_with_case_insensitive_impl(const char *text, const char *prefix) {
    while (*prefix) {
        if (*text == '\0') return 0;
        char a = (char)tolower((unsigned char)*text);
        char b = (char)tolower((unsigned char)*prefix);
        if (a != b) return 0;
        text++; prefix++;
    }
    return 1;
}

bool str_starts_with_case_insensitive(const char *text, const char *prefix) {
    if (!text || !prefix) return false;
    return starts_with_case_insensitive_impl(text, prefix) ? true : false;
}

int read_line(char *buf, int size) {
    if (!fgets(buf, size, stdin)) return 0;
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
    return 1;
}

void pause_prompt(void) {
    printf("Pressione ENTER para continuar...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

int safe_atoi(const char *s, int *out) {
    if (!s || !*s) return 0;
    int sign = 1;
    const char *p = s;
    if (*p == '-') { sign = -1; p++; }
    if (!isdigit((unsigned char)*p)) return 0;
    int val = 0;
    while (*p) {
        if (!isdigit((unsigned char)*p)) return 0;
        int d = *p - '0';
        if (val > (2147483647 - d) / 10) return 0; // overflow
        val = val * 10 + d;
        p++;
    }
    *out = val * sign;
    return 1;
}

// --- UTF-8 helpers para alinhamento ---

// Retorna número de bytes do próximo code point UTF-8 a partir de p.
// Retorna 1 para ASCII, 2~4 para multibyte, ou 1 se inválido (fallback).
static int utf8_cp_bytes(const unsigned char *p) {
    if (*p < 0x80) return 1;
    if ((*p & 0xE0) == 0xC0) return 2;
    if ((*p & 0xF0) == 0xE0) return 3;
    if ((*p & 0xF8) == 0xF0) return 4;
    // byte inválido, trata como 1
    return 1;
}

int utf8_len(const char *s) {
    if (!s) return 0;
    int count = 0;
    const unsigned char *p = (const unsigned char*)s;
    while (*p) {
        int n = utf8_cp_bytes(p);
        p += n;
        count++;
    }
    return count;
}

// Copia no máximo 'width' code points de 's' para 'dst' (cap bytes), retorna bytes escritos.
static int utf8_copy_n_cps(char *dst, int cap, const char *s, int width) {
    int bytes = 0, cps = 0;
    const unsigned char *p = (const unsigned char*)s;
    while (*p && cps < width) {
        int n = utf8_cp_bytes(p);
        if (bytes + n >= cap) break;
        for (int i = 0; i < n; i++) dst[bytes++] = (char)p[i];
        p += n;
        cps++;
    }
    if (bytes < cap) dst[bytes] = '\0';
    return bytes;
}

void print_utf8_padded(const char *s, int width) {
    if (!s) s = "";
    int vis = utf8_len(s);
    if (vis == width) {
        fputs(s, stdout);
        return;
    }
    if (vis < width) {
        // imprime e completa com espaços
        fputs(s, stdout);
        for (int i = 0; i < width - vis; i++) putchar(' ');
        return;
    }
    // vis > width: truncar e colocar '…' no final
    // reservamos 1 para '…'
    int keep = width - 1;
    if (keep < 0) keep = 0;
    char buf[256];
    utf8_copy_n_cps(buf, sizeof(buf), s, keep);
    fputs(buf, stdout);
    putchar((char)0xE2); putchar((char)0x80); putchar((char)0xA6); // '…' U+2026
}
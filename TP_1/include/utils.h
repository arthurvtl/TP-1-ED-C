#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdbool.h>

void str_trim(char *s);
void str_to_lower(char *s);
bool str_starts_with_case_insensitive(const char *text, const char *prefix);
int read_line(char *buf, int size);
void pause_prompt(void);
int safe_atoi(const char *s, int *out);


// Largura visual (em code points) de uma string UTF-8 simples (sem combinar).
int utf8_len(const char *s);

// Imprime texto UTF-8 truncando/padronizando para largura visual fixa.
// Se o texto for maior que width, trunca com '…' (ellipsis).
void print_utf8_padded(const char *s, int width);

#endif
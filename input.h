#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <stdlib.h>

typedef struct Config Config;
typedef struct Vento Vento;
typedef struct Foco Foco;
typedef struct ZonaContencao ZonaContencao;
typedef struct Entrada Entrada;

int ler_entrada(const char *nome_arquivo, Entrada *entrada);
void imprimir_entrada(const Entrada *entrada);
void liberar_entrada(Entrada *entrada);

#endif // INPUT_H
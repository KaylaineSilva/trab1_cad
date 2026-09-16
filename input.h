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
int get_value(Entrada *entrada, char tipo);

Foco get_foco(Entrada *entrada, int indice);
int get_foco_linha(Foco *foco);
int get_foco_coluna(Foco *foco);


void liberar_entrada(Entrada *entrada);

#endif // INPUT_H
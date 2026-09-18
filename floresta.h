#ifndef FLORESTA_H
#define FLORESTA_H

#include "input.h"

typedef struct {
    int *cobertura;
    int *umidade;

    int *estado_atual;
    int *proximo_estado;

    int *tempo_atual_queima;
    int *proximo_tempo_queima;

    int *ativacao;
} Floresta;

int criar_floresta(const Entrada *entrada, Floresta *floresta);
void liberar_floresta(Floresta *floresta);

void imprimir_floresta(const Floresta *floresta,int linhas,int colunas);

#endif
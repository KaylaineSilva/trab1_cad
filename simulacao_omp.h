#ifndef SIMULACAO_OMP_H
#define SIMULACAO_OMP_H

#include "input.h"
#include "floresta.h"

#define NAO_COMBUSTIVEL 0
#define INTACTA 1
#define EM_CHAMAS 2
#define QUEIMADA 3
#define CONTENCAO 4



/*
Entrada é const porque nao deve ser alterada
entrada: paramentros da simulacao: passos, threads, limiar, vento etc
floresta: vetor de celulas que representa a floresta
*/
void simular_paralelo(const Entrada *entrada, Floresta *floresta);

#endif 
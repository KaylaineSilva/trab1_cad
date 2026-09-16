#ifndef SIMULACAO_OMP_H
#define SIMULACAO_OMP_H

#include "input.h"
#include "floresta.h"


/*
Entrada é const porque nao deve ser alterada
entrada: paramentros da simulacao: passos, threads, limiar, vento etc
floresta: vetor de celulas que representa a floresta
*/
void simular_paralelo(const Entrada *entrada, Celula *floresta);

#endif 
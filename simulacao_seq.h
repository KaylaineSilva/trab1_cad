#ifndef SIMULACAO_SEQ_H
#define SIMULACAO_SEQ_H

#include "input.h"
#include "floresta.h"

#define NAO_COMBUSTIVEL 0
#define INTACTA 1
#define EM_CHAMAS 2
#define QUEIMADA 3
#define CONTENCAO 4

#define AGUA 0
#define SOLO  1
#define RASTEIRA 2
#define FLORESTA 3

typedef struct {
    long long nao_combustiveis;
    long long intactas;
    long long em_chamas;
    long long queimadas;
    long long contencao;
    long long novas_ignicoes;
} EstatisticasPasso;

typedef struct {
    int passos_executados;

    int passo_pico;
    long long pico_ignicoes;

    long long int nao_combustiveis;
    long long int intactas;
    long long int em_chamas;
    long long int queimadas;
    long long int contencao;
    long long int total_ignicoes;
    double percentual_queimado;
    double percentual_protegido;
    unsigned long long checksum;
    double tempo;

} ResultadoSimulacao;

/*
Entrada é const porque nao deve ser alterada
entrada: paramentros da simulacao: passos, threads (nao usado), limiar, vento etc
floresta: vetor de celulas que representa a floresta
*/
void simular_sequencial(const Entrada *entrada, Floresta *floresta, ResultadoSimulacao *resultado);
void imprimir_resultados(ResultadoSimulacao *resultado);

#endif

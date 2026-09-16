#include <omp.h>

#include "simulacao_omp.h"

void simular_paralelo(const Entrada *entrada, Floresta *floresta){

    int linhas = entrada->config.linhas;
    int colunas = entrada->config.colunas;
    int passos = entrada->config.passos;
    int threads = entrada->config.threads; 

    int total_celulas = linhas * colulas;

    //diz ao OpenMP quantas threads ele deve usar nas regioes paralelas
    omp_set_num_threads(threads);

    #pragma omp parallel
    {
        for (int passo = 0 )
    }

    //regiao paralela
}
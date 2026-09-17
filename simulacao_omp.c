#include <omp.h>

#include "simulacao_omp.h"

/*void simular_paralelo(const Entrada *entrada, Floresta *floresta){

    int linhas = entrada->config.linhas;
    int colunas = entrada->config.colunas;
    int passos = entrada->config.passos;
    int threads = entrada->config.threads; 

    int total_celulas = linhas * colunas;

    //diz ao OpenMP quantas threads ele deve usar nas regioes paralelas
    omp_set_num_threads(threads);

    #pragma omp parallel
    {
        for (int passo = 0 )
    }

    //regiao paralela
}*/

void ativar_zonas(const Entrada *entrada, Floresta *floresta, int passo){
    long long int linha = entrada->config.linhas;
    long long int coluna = entrada->config.colunas;
    
    for(long long int i=0; i<linha; i++){
        for (long long int j=0; j<coluna; j++){
            
            long long int indice = i*coluna+j;
            
            if(floresta->ativacao[indice]==passo){
                switch (floresta->estado_atual[indice]){
                case INTACTA:
                    floresta->ativacao[indice] = CONTENCAO;
                    break;
                case EM_CHAMAS:
                    floresta->estado_atual[indice] = EM_CHAMAS;
                    break;
                case QUEIMADA:
                    floresta->estado_atual[indice] = QUEIMADA;
                    break;
                case NAO_COMBUSTIVEL:
                    floresta->estado_atual[indice] = NAO_COMBUSTIVEL;
                    break;
                case CONTENCAO:
                    floresta->estado_atual[indice] = CONTENCAO;
                    break;
                default:
                    break;
                }
            }
        }
    }
}

int pega_fogo(const Entrada *entrada, Floresta *floresta, Floresta *celula_atual, long long int indice_atual){

    //Sentido propagação do fogo
    long long int prop_linha = 
}
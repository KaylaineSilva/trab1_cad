#include <omp.h>

#include "simulacao_omp.h"

//prototipos
void simular_paralelo(const Entrada *entrada, Floresta *floresta, ResultadoSimulacao *resultado);
long long contar_combustiveis(const Floresta *floresta, long long total);
void ativar_zonas(const Entrada *entrada, Floresta *floresta, int passo);
int calcular_potencial(const Entrada *entrada, const Floresta *floresta, long long int linha, long long int coluna);
int fator_combustivel(int cobertura);
int pega_fogo(const Entrada *entrada, Floresta *floresta, long long int linha, long long int coluna);
int atualizar_celula(const Entrada *entrada, Floresta *floresta, long long indice);
void calcular_estatisticas(const Floresta *floresta, long long total, EstatisticasPasso *stats);
void trocar_matrizes(Floresta *floresta);
void imprimir_resultados(ResultadoSimulacao *resultado);


void simular_paralelo(const Entrada *entrada, Floresta *floresta, ResultadoSimulacao *resultado) {
    int linhas = entrada->config.linhas;
    int colunas = entrada->config.colunas;
    int passos = entrada->config.passos;
    int threads = entrada->config.threads;

    long long total_celulas = (long long) linhas * colunas;

    //inicializar
    resultado->passos_executados = 0;
    resultado->total_ignicoes = 0;
    resultado->passo_pico = -1;
    resultado->pico_ignicoes = 0;

    int passo = 0;
    int continuar = 1;

    long long novas_ignicoes = 0;

    long long nao_combustiveis = 0;
    long long intactas = 0;
    long long em_chamas = 0;
    long long queimadas = 0;
    long long contencao = 0;

    EstatisticasPasso stats = {0};
    long long combustiveis_iniciais = contar_combustiveis(floresta, total_celulas);

    double inicio = omp_get_wtime();

    #pragma omp parallel num_threads(threads) default(none) shared(entrada, floresta, resultado, total_celulas, passos, passo, continuar, novas_ignicoes, nao_combustiveis, intactas, em_chamas, queimadas, contencao, stats)
    {
        while (continuar && passo<passos) {

            /*Reinicia os contadores do passo*/
            #pragma omp single
            {
                novas_ignicoes = 0;

                nao_combustiveis = 0;
                intactas = 0;
                em_chamas = 0;
                queimadas = 0;
                contencao = 0;
            }

            /*ativação das zonas*/
            #pragma omp for simd schedule(static)
            for (long long i = 0; i < total_celulas; i++) {

                if (floresta->ativacao[i] == passo &&
                    floresta->estado_atual[i] == INTACTA) {

                    floresta->estado_atual[i] = CONTENCAO;
                }
            }

            /*barreira implicita aqui
            todas as contencoes ja foram aplicadas antes do calculo do prox estado
             */

            /*próximo estado*/
            #pragma omp for schedule(static) reduction(+:novas_ignicoes)
            for (long long i = 0; i < total_celulas; i++) {
                novas_ignicoes += atualizar_celula(entrada, floresta, i);
            }

            /*outra barreira implícita
             proximo_estado inteiro já está pronto
             */

            /*estísticas do próximo estado*/
            #pragma omp for simd schedule(static) reduction(+:nao_combustiveis, intactas, em_chamas, queimadas, contencao)
            for (long long i = 0; i < total_celulas; i++) {

                int estado = floresta->proximo_estado[i];

                nao_combustiveis += (estado == NAO_COMBUSTIVEL);
                intactas         += (estado == INTACTA);
                em_chamas        += (estado == EM_CHAMAS);
                queimadas        += (estado == QUEIMADA);
                contencao        += (estado == CONTENCAO);
            }

            /*só UMA thread mexe nos resultados globais e troca os ponteiros*/
            #pragma omp single
            {
                stats.nao_combustiveis = nao_combustiveis;

                stats.intactas = intactas;

                stats.em_chamas = em_chamas;

                stats.queimadas = queimadas;

                stats.contencao = contencao;

                stats.novas_ignicoes = novas_ignicoes;

                /* resultados acumulado*/
                resultado->total_ignicoes += novas_ignicoes;


                /*Pico
                Usa > e nao >= para preservar o primeiro passo em caso de empate
                 */
                if (novas_ignicoes >
                    resultado->pico_ignicoes) {

                    resultado->pico_ignicoes = novas_ignicoes;

                    resultado->passo_pico = passo;
                }


                /* troca atual <-> proximo
                 */
                trocar_matrizes(floresta);


                resultado->passos_executados++;

                /*
                 *condição de parada
                 */
                if (stats.em_chamas == 0) {
                    continuar = 0;
                }

                passo++;
            }
        }
    }

    double fim = omp_get_wtime();

    resultado->tempo = fim - inicio;


    /*o ultimo stats calculado corresponde ao estado que agora esta em estado_atual
     */
    if (resultado->passos_executados > 0) {

        resultado->nao_combustiveis = stats.nao_combustiveis;

        resultado->intactas = stats.intactas;

        resultado->em_chamas = stats.em_chamas;

        resultado->queimadas = stats.queimadas;

        resultado->contencao = stats.contencao;
    }

    if (combustiveis_iniciais == 0) {
        resultado->percentual_queimado = 0.0;
        resultado->percentual_protegido = 0.0;
    } else {
        resultado->percentual_queimado = 100.0 * (resultado->queimadas + resultado->em_chamas) / combustiveis_iniciais;
        resultado->percentual_protegido = 100.0 *resultado->contencao / combustiveis_iniciais;
    }

    resultado->checksum = 0;

    for (long long i = 0; i < total_celulas; i++) {

        resultado->checksum = resultado->checksum * 31ULL + (unsigned long long) floresta->estado_atual[i];

        resultado->checksum =
            resultado->checksum * 31ULL +
            (unsigned long long)
            floresta->tempo_atual_queima[i];
    }
}

long long contar_combustiveis(const Floresta *floresta, long long total) {
    long long quantidade = 0;

    for (long long i = 0; i < total; i++) {

        if (floresta->cobertura[i] == RASTEIRA || floresta->cobertura[i] == FLORESTA) {
            quantidade++;
        }
    }

    return quantidade;
}

void ativar_zonas(const Entrada *entrada, Floresta *floresta, int passo){
    long long int linha = entrada->config.linhas;
    long long int coluna = entrada->config.colunas;
    
    for(long long int i=0; i<linha; i++){
        for (long long int j=0; j<coluna; j++){
            
            long long int indice = i*coluna+j;
            
            if(floresta->ativacao[indice]==passo && floresta->estado_atual[indice] == INTACTA){
                floresta->estado_atual[indice] = CONTENCAO;    
            }
        }
    }
}

int fator_combustivel(int cobertura) {
    if (cobertura == RASTEIRA) {
        return 8;
    }

    if (cobertura == FLORESTA) {
        return 12;
    }

    return 0;
}

int calcular_potencial(const Entrada *entrada, const Floresta *floresta, long long int linha, long long int coluna){
    int L = entrada->config.linhas;
    int C = entrada->config.colunas;

    int soma_pesos = 0;

    for(int dl=-1; dl<=1; dl++){
        for (int dc=-1; dc<=1; dc++) {
            //pular a própria célula
            if(dl==0 && dc==0) {
                continue;
            }

            int linha_vizinho = linha + dl;
            int coluna_vizinho = coluna + dc;

            //verificando se o vizinho é fora da matriz
            if(linha_vizinho<0 || linha_vizinho>=L || coluna_vizinho<0 || coluna_vizinho>=C){
                continue;
            }

            long long indice_vizinho = (long long) linha_vizinho * C + coluna_vizinho;
        
            // apenas vizinhos em chamas contribuem
            if (floresta->estado_atual[indice_vizinho] != EM_CHAMAS) {
                continue;
            }

            int prop_linha = linha - linha_vizinho;
            int prop_coluna = coluna - coluna_vizinho;

            int peso_basico;

            if (abs(prop_linha) + abs(prop_coluna) == 1) {
                peso_basico = 10;
            } else {
                peso_basico = 7;
            }

            int alinhamento =
                prop_linha * entrada->vento.linha +
                prop_coluna * entrada->vento.coluna;

            int peso =
                peso_basico +
                entrada->vento.intensidade * alinhamento;

            if (peso < 1) {
                peso = 1;
            }

            soma_pesos += peso;
        }
    }

    long long indice = (long long) linha * C + coluna;

    int fator = fator_combustivel(floresta->cobertura[indice]);

    int potencial = soma_pesos* fator *(100 - floresta->umidade[indice])/100;

    return potencial;
}

int pega_fogo(const Entrada *entrada, Floresta *floresta, long long int linha, long long int coluna){
    return calcular_potencial(entrada, floresta, linha, coluna)>=entrada->config.limiar;
}

int atualizar_celula(const Entrada *entrada, Floresta *floresta, long long indice) {
    int C = entrada->config.colunas;

    int linha = indice / C;
    int coluna = indice % C;

    int estado = floresta->estado_atual[indice];

    switch (estado) {
        case NAO_COMBUSTIVEL:
            floresta->proximo_estado[indice] = NAO_COMBUSTIVEL;
            floresta->proximo_tempo_queima[indice] = 0;
            return 0;

        case INTACTA:
            if (pega_fogo(entrada, floresta, linha, coluna)) {

                floresta->proximo_estado[indice] = EM_CHAMAS;

                if (floresta->cobertura[indice] == RASTEIRA) {
                    floresta->proximo_tempo_queima[indice] = 2;
                } else {
                    floresta->proximo_tempo_queima[indice] = 4;
                }

                return 1;
            }

            floresta->proximo_estado[indice] = INTACTA;
            floresta->proximo_tempo_queima[indice] = 0;
            return 0;

        case EM_CHAMAS: {
            int novo_tempo =
                floresta->tempo_atual_queima[indice] - 1;

            if (novo_tempo == 0) {
                floresta->proximo_estado[indice] = QUEIMADA;
                floresta->proximo_tempo_queima[indice] = 0;
            } else {
                floresta->proximo_estado[indice] = EM_CHAMAS;
                floresta->proximo_tempo_queima[indice] = novo_tempo;
            }

            return 0;
        }

        case QUEIMADA:
            floresta->proximo_estado[indice] = QUEIMADA;
            floresta->proximo_tempo_queima[indice] = 0;
            return 0;

        case CONTENCAO:
            floresta->proximo_estado[indice] = CONTENCAO;
            floresta->proximo_tempo_queima[indice] = 0;
            return 0;
    }

    return 0;
}

void calcular_estatisticas(const Floresta *floresta, long long total, EstatisticasPasso *stats) {
    stats->nao_combustiveis = 0;
    stats->intactas = 0;
    stats->em_chamas = 0;
    stats->queimadas = 0;
    stats->contencao = 0;

    for (long long i = 0; i < total; i++) {

        switch (floresta->proximo_estado[i]) {
            case NAO_COMBUSTIVEL:
                stats->nao_combustiveis++;
                break;

            case INTACTA:
                stats->intactas++;
                break;

            case EM_CHAMAS:
                stats->em_chamas++;
                break;

            case QUEIMADA:
                stats->queimadas++;
                break;

            case CONTENCAO:
                stats->contencao++;
                break;
        }
    }
}

void trocar_matrizes(Floresta *floresta) {
    int *aux;

    aux = floresta->estado_atual;
    floresta->estado_atual = floresta->proximo_estado;
    floresta->proximo_estado = aux;

    aux = floresta->tempo_atual_queima;
    floresta->tempo_atual_queima = floresta->proximo_tempo_queima;
    floresta->proximo_tempo_queima = aux;
}

void imprimir_resultados(ResultadoSimulacao *resultado) {
    printf("passos: %d\n", resultado->passos_executados);

    printf("nao_combustiveis: %lld\n", resultado->nao_combustiveis);

    printf("intactas: %lld\n", resultado->intactas);

    printf("em_chamas: %lld\n", resultado->em_chamas);

    printf("queimadas: %lld\n", resultado->queimadas);

    printf("contencao: %lld\n",resultado->contencao);

    printf("total_ignicoes: %lld\n", resultado->total_ignicoes);

    printf("pico_ignicoes: %d %lld\n",resultado->passo_pico, resultado->pico_ignicoes);

    printf("percentual_queimado: %.2f\n", resultado->percentual_queimado);

    printf("percentual_protegido: %.2f\n", resultado->percentual_protegido);

    printf("checksum: %llu\n", resultado->checksum);

    printf("tempo: %.6f\n", resultado->tempo);
}
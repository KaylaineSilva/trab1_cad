#include <omp.h> /* usado apenas para omp_get_wtime(), conforme item 12 do enunciado */

#include "simulacao_seq.h"

/* prototipos */
void simular_sequencial(const Entrada *entrada, Floresta *floresta, ResultadoSimulacao *resultado);
long long contar_combustiveis(const Floresta *floresta, long long total);
void ativar_zonas(const Entrada *entrada, Floresta *floresta, int passo);
int fator_combustivel(int cobertura);
int calcular_potencial(const Entrada *entrada, const Floresta *floresta, long long int linha, long long int coluna);
int pega_fogo(const Entrada *entrada, Floresta *floresta, long long int linha, long long int coluna);
int atualizar_celula(const Entrada *entrada, Floresta *floresta, long long indice);
void calcular_estatisticas_estado(const int *estado, long long total, EstatisticasPasso *stats);
void trocar_matrizes(Floresta *floresta);
void imprimir_resultados(ResultadoSimulacao *resultado);

void simular_sequencial(const Entrada *entrada, Floresta *floresta, ResultadoSimulacao *resultado) {
    long long total_celulas =
        (long long) entrada->config.linhas * entrada->config.colunas;

    int passos = entrada->config.passos;

    // inicializando
    resultado->passos_executados = 0;
    resultado->total_ignicoes = 0;
    resultado->passo_pico = -1;
    resultado->pico_ignicoes = 0;

    long long combustiveis_iniciais = contar_combustiveis(floresta, total_celulas);

    EstatisticasPasso stats_iniciais = {0};
    calcular_estatisticas_estado(floresta->estado_atual, total_celulas, &stats_iniciais);

    int continuar = (stats_iniciais.em_chamas > 0);
    int passo = 0;

    EstatisticasPasso stats = {0};

    double inicio = omp_get_wtime();

    while (continuar && passo < passos) {

        long long novas_ignicoes = 0;

        for (long long i = 0; i < total_celulas; i++) {
            if (floresta->ativacao[i] == passo &&
                floresta->estado_atual[i] == INTACTA) {

                floresta->estado_atual[i] = CONTENCAO;
            }
        }

        for (long long i = 0; i < total_celulas; i++) {
            novas_ignicoes += atualizar_celula(entrada, floresta, i);
        }

        calcular_estatisticas_estado(floresta->proximo_estado, total_celulas, &stats);
        stats.novas_ignicoes = novas_ignicoes;

        resultado->total_ignicoes += novas_ignicoes;

        /* pico de ignições: usa > (não >=) para preservar o primeiro passo em empate */
        if (novas_ignicoes > resultado->pico_ignicoes) {
            resultado->pico_ignicoes = novas_ignicoes;
            resultado->passo_pico = passo;
        }

        /* trocar as matrizes (atual <-> próximo) */
        trocar_matrizes(floresta);

        resultado->passos_executados++;

        /* condição de parada */
        if (stats.em_chamas == 0) {
            continuar = 0;
        }

        passo++;
    }

    double fim = omp_get_wtime();
    resultado->tempo = fim - inicio;

    if (resultado->passos_executados > 0) {
        // o ultimo stats calculado é o estado_atual
        resultado->nao_combustiveis = stats.nao_combustiveis;
        resultado->intactas = stats.intactas;
        resultado->em_chamas = stats.em_chamas;
        resultado->queimadas = stats.queimadas;
        resultado->contencao = stats.contencao;
    } else {
        // nenhum passo executado: as estatísticas finais são as do estado inicial
        resultado->nao_combustiveis = stats_iniciais.nao_combustiveis;
        resultado->intactas = stats_iniciais.intactas;
        resultado->em_chamas = stats_iniciais.em_chamas;
        resultado->queimadas = stats_iniciais.queimadas;
        resultado->contencao = stats_iniciais.contencao;
    }

    if (combustiveis_iniciais == 0) {
        resultado->percentual_queimado = 0.0;
        resultado->percentual_protegido = 0.0;
    } else {
        resultado->percentual_queimado =
            100.0 * (resultado->queimadas + resultado->em_chamas) / combustiveis_iniciais;

        resultado->percentual_protegido =
            100.0 * resultado->contencao / combustiveis_iniciais;
    }

    /* checksum: sempre sequencial e na ordem linear da matriz, fora do trecho cronometrado */
    resultado->checksum = 0;

    for (long long i = 0; i < total_celulas; i++) {
        resultado->checksum =
            resultado->checksum * 31ULL + (unsigned long long) floresta->estado_atual[i];

        resultado->checksum =
            resultado->checksum * 31ULL + (unsigned long long) floresta->tempo_atual_queima[i];
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

void ativar_zonas(const Entrada *entrada, Floresta *floresta, int passo) {
    long long int linha = entrada->config.linhas;
    long long int coluna = entrada->config.colunas;

    for (long long int i = 0; i < linha; i++) {
        for (long long int j = 0; j < coluna; j++) {

            long long int indice = i * coluna + j;

            if (floresta->ativacao[indice] == passo && floresta->estado_atual[indice] == INTACTA) {
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

int calcular_potencial(const Entrada *entrada, const Floresta *floresta, long long int linha, long long int coluna) {
    int L = entrada->config.linhas;
    int C = entrada->config.colunas;

    int soma_pesos = 0;

    for (int dl = -1; dl <= 1; dl++) {
        for (int dc = -1; dc <= 1; dc++) {
            /* pular a própria célula */
            if (dl == 0 && dc == 0) {
                continue;
            }

            int linha_vizinho = linha + dl;
            int coluna_vizinho = coluna + dc;

            /* vizinho fora da matriz é ignorado */
            if (linha_vizinho < 0 || linha_vizinho >= L || coluna_vizinho < 0 || coluna_vizinho >= C) {
                continue;
            }

            long long indice_vizinho = (long long) linha_vizinho * C + coluna_vizinho;

            /* somente vizinhos em chamas contribuem */
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

            int peso = peso_basico + entrada->vento.intensidade * alinhamento;

            if (peso < 1) {
                peso = 1;
            }

            soma_pesos += peso;
        }
    }

    long long indice = (long long) linha * C + coluna;

    int fator = fator_combustivel(floresta->cobertura[indice]);

    int potencial = soma_pesos * fator * (100 - floresta->umidade[indice]) / 100;

    return potencial;
}

int pega_fogo(const Entrada *entrada, Floresta *floresta, long long int linha, long long int coluna) {
    return calcular_potencial(entrada, floresta, linha, coluna) >= entrada->config.limiar;
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
            int novo_tempo = floresta->tempo_atual_queima[indice] - 1;

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

void calcular_estatisticas_estado(const int *estado, long long total, EstatisticasPasso *stats) {
    stats->nao_combustiveis = 0;
    stats->intactas = 0;
    stats->em_chamas = 0;
    stats->queimadas = 0;
    stats->contencao = 0;
    stats->novas_ignicoes = 0;

    for (long long i = 0; i < total; i++) {
        switch (estado[i]) {
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

    printf("contencao: %lld\n", resultado->contencao);

    printf("total_ignicoes: %lld\n", resultado->total_ignicoes);

    printf("pico_ignicoes: %d %lld\n", resultado->passo_pico, resultado->pico_ignicoes);

    printf("percentual_queimado: %.2f\n", resultado->percentual_queimado);

    printf("percentual_protegido: %.2f\n", resultado->percentual_protegido);

    printf("checksum: %llu\n", resultado->checksum);

    printf("tempo: %.6f\n", resultado->tempo);
}

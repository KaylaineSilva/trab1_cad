#include <stdlib.h>
#include <stdio.h>
#include "input.h"

// Definição das structs 

// Struct para a configuração da matriz de simulação
struct Config {
    int linhas;
    int colunas;
    int passos;
    int threads;
    unsigned int seed;
    int limiar;
};

// struct para representar o vento
struct Vento {
    int linha;
    int coluna;
    int intensidade;
};

struct Foco {
    int linha;
    int coluna;
};
 
struct ZonaContencao {
    int passo_ativacao;
    int linha_inicio;
    int coluna_inicio;
    int linha_fim;
    int coluna_fim;
};

struct Entrada{
    Config config;
    Vento vento;

    int qtd_focos;
    Foco *focos;

    int qtd_zonas;
    ZonaContencao *zonas;
};

void imprimir_configuracao(const Config *config) {
    printf("=== CONFIGURACAO ===\n");
    printf("Linhas: %d\n", config->linhas);
    printf("Colunas: %d\n", config->colunas);
    printf("Passos: %d\n", config->passos);
    printf("Threads: %d\n", config->threads);
    printf("Seed: %u\n", config->seed);
    printf("Limiar: %d\n", config->limiar);
}

void imprimir_vento(const Vento *vento) {
    printf("=== VENTO ===\n");
    printf("Linha: %d\n", vento->linha);
    printf("Coluna: %d\n", vento->coluna);
    printf("Intensidade: %d\n", vento->intensidade);
}

void imprimir_focos(const Entrada *entrada) {
    printf("=== FOCOS ===\n");
    printf("Quantidade: %d\n", entrada->qtd_focos);

    for (int i = 0; i < entrada->qtd_focos; i++) {
        printf("Foco %d: (%d, %d)\n",
               i,
               entrada->focos[i].linha,
               entrada->focos[i].coluna);
    }
}

void imprimir_zonas(const Entrada *entrada) {
    printf("=== ZONAS DE CONTENCAO ===\n");
    printf("Quantidade: %d\n", entrada->qtd_zonas);

    for (int i = 0; i < entrada->qtd_zonas; i++) {
        const ZonaContencao *zona = &entrada->zonas[i];

        printf(
            "Zona %d: passo=%d, inicio=(%d, %d), fim=(%d, %d)\n",
            i,
            zona->passo_ativacao,
            zona->linha_inicio,
            zona->coluna_inicio,
            zona->linha_fim,
            zona->coluna_fim
        );
    }
}

void imprimir_entrada(const Entrada *entrada) {
    imprimir_configuracao(&entrada->config);
    printf("\n");

    imprimir_vento(&entrada->vento);
    printf("\n");

    imprimir_focos(entrada);
    printf("\n");

    imprimir_zonas(entrada);
}

/*
Função: 
Descrição:
Saída: 
*/
int ler_entrada(const char *nome_arquivo, Entrada *entrada){
    int f, z;

    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo de entrada: %s\n", nome_arquivo);
        return 0;
    }

    // 1. ler configuração
    fscanf(arquivo, "%d %d %d %d %u %d",
           &entrada->config.linhas,
           &entrada->config.colunas,
           &entrada->config.passos,
           &entrada->config.threads,
           &entrada->config.seed,
           &entrada->config.limiar);
    
    printf("Configuração lida: linhas=%d, colunas=%d, passos=%d, threads=%d, seed=%u, limiar=%d\n",
           entrada->config.linhas,
           entrada->config.colunas,
           entrada->config.passos,
           entrada->config.threads,
           entrada->config.seed,
           entrada->config.limiar);

    // 2. validar configuração
    if(entrada->config.linhas <= 0 || entrada->config.colunas <= 0 || entrada->config.passos < 0 || entrada->config.threads <= 0) {
        fprintf(stderr, "Configuração inválida: linhas, colunas, passos e threads devem ser positivos.\n");
        fclose(arquivo);
        return 0;
    }

    // 3. ler vento
    fscanf(arquivo, "%d %d %d",
           &entrada->vento.linha,
           &entrada->vento.coluna,
           &entrada->vento.intensidade);

    printf("Vento lido: linha=%d, coluna=%d, intensidade=%d\n",
           entrada->vento.linha,
           entrada->vento.coluna,
           entrada->vento.intensidade);
    // 4. validar vento
    if((entrada->vento.linha<-1 || entrada->vento.linha>1) || (entrada->vento.coluna<-1 || entrada->vento.coluna>1) || (entrada->vento.linha==0 && entrada->vento.coluna==0) || (entrada->vento.intensidade<0 || entrada->vento.intensidade>5)) {
        fprintf(stderr, "Vento inválido: linha e coluna devem estar entre -1 e 1, e não podem ser ambos zero. E intensidade deve estar entre 0 e 5.\n");
        fclose(arquivo);
        return 0;
    }

    // 5. ler F e Z
    fscanf(arquivo, "%d", &f);
    fscanf(arquivo, "%d", &z);

    entrada->qtd_focos=f;
    entrada->qtd_zonas=z;

    printf("F: %d; Z: %d\n", f, z);

    if(f<0 || z<0){
        fprintf(stderr, "Quantidade de foco e de zonas de contenção devem ser maiores ou iguais a zero.\n");
        fclose(arquivo);
        return 0;
    }

    // 6. alocar focos
    // entrada->focos = malloc(...)
    entrada->focos = malloc(f * sizeof(Foco));

    // 7. ler e validar focos
    for (int i=0; i<f; i++){

        fscanf(arquivo, "%d %d", 
            &entrada->focos[i].linha, 
            &entrada->focos[i].coluna);
    }

    //8. verificação dos focos 
    unsigned char *foco_usado = calloc(entrada->config.linhas*entrada->config.colunas, sizeof(unsigned char));

    for (int i=0; i<f; i++){
        int linha = entrada->focos[i].linha;
        int coluna = entrada->focos[i].coluna;

        long long indice = (long long) linha * entrada->config.colunas + coluna;
    
        if(foco_usado[indice]){
            fprintf(stderr, "Foco repetido.\n");
            free(foco_usado);
            fclose(arquivo);
            return 0;
        }

        foco_usado[indice] = 1;
    }   

    //Nenhum foco repetido
    free(foco_usado);

    // 9. alocar zonas
    // entrada->zonas = malloc(...)
    entrada->zonas = malloc(z * sizeof(ZonaContencao));

    for (int i=0; i<z; i++) {
        fscanf(arquivo, "%d %d %d %d %d",
        &entrada->zonas[i].passo_ativacao,
        &entrada->zonas[i].linha_inicio,
        &entrada->zonas[i].coluna_inicio,
        &entrada->zonas[i].linha_fim,
        &entrada->zonas[i].coluna_fim);
    }

    // 10. ler e validar zonas
    // As verificações possíveis durante a leitura são: ausência de focos repetidos, limites iniciais não superiores aos finais, e 0 ≤ passo_ativacao < P
    for (int i=0; i<z; i++){
        ZonaContencao *zona = &entrada->zonas[i];

        //passo de ativação deve estar em [0, P-1]
        if(zona->passo_ativacao<0 || zona->passo_ativacao>=entrada->config.passos){
            fprintf(stderr, "Passos de ativação inválidos, o valor deve ser maior ou igual a zero e menor que o número máximo de passos da simulação.\n");
            fclose(arquivo);
            return 0;
        }

        //limite inicial nao pode ser maior que o final
        if(zona->linha_inicio>zona->linha_fim || zona->coluna_inicio>zona->coluna_fim){
            fprintf(stderr, "Limites invalidos na zona de contencao.\n");
            fclose(arquivo);
            return 0;
        }

        //zona deve estar dentro dos limites da matriz
        if(zona->linha_inicio<0 || zona->linha_fim>=entrada->config.linhas || zona->coluna_inicio<0 || zona->coluna_fim>=entrada->config.colunas){
            fprintf(stderr, "Zona de contencao fora da matriz.\n");
            fclose(arquivo);
            return 0;
        }

    }

    fclose(arquivo);

    

    return 1; //leitura bem sucedida
}

/*
Função:
Descrição:
Saída: sem saída. 
*/
void liberar_entrada(Entrada *entrada) {
    if (entrada != NULL) {
        //liberar as matrizes alocadas dinamicamente
        
        free(entrada->focos);
        free(entrada->zonas);
    }
}
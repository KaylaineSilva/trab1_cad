#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <stdlib.h>

// Definição das structs 

// Struct para a configuração da matriz de simulação
typedef struct  {
    int linhas;
    int colunas;
    int passos;
    int threads;
    unsigned int seed;
    int limiar;
} Config;

// struct para representar o vento
typedef struct {
    int linha;
    int coluna;
    int intensidade;
} Vento;

typedef struct {
    int linha;
    int coluna;
} Foco;
 
typedef struct {
    int passo_ativacao;
    int linha_inicio;
    int coluna_inicio;
    int linha_fim;
    int coluna_fim;
} ZonaContencao;

typedef struct {
    Config config;
    Vento vento;

    int qtd_focos;
    Foco *focos;

    int qtd_zonas;
    ZonaContencao *zonas;
} Entrada;

int ler_entrada(const char *nome_arquivo, Entrada *entrada);
void imprimir_entrada(const Entrada *entrada);
void liberar_entrada(Entrada *entrada);

#endif // INPUT_H
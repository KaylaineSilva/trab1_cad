//orquestra o programa
#include <stdio.h>
#include <stdlib.h>

#include "input.h"
#include "floresta.h"
#include "simulacao_omp.h"

/*
argc: quantidade de argumentos passados no terminal
argv: vetor contendo esses argumentos

./fire_omp entrada.txt 
argc ==2
argv[0] = "./fire_omp"
argv[1] = "entrada.txt"
*/
int main(int argc, char *argv[]) {

    //verifica argumentos da linha de comando
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo_de_entrada>\n", argv[0]);
        return EXIT_FAILURE; //programa termina com erro
    }

    //ler a entrada
    Entrada entrada;
    if (!ler_entrada(argv[1], &entrada)) {
        return EXIT_FAILURE;
    }

    imprimir_entrada(&entrada);

    //Usar a entrada para montar a floresta (matriz)
    Celula *floresta = NULL;

    if (!criar_floresta(&entrada, &floresta)){
        liberar_entrada(&entrada);
        return EXIT_FAILURE;
    }

    // Rodar a simulação 
    simular_paralelo(&entrada, floresta);

    // Calcular/imprimir os resultados


    //liberar memoria
    liberar_floresta(&floresta);
    liberar_entrada(&entrada);

    return EXIT_SUCCESS;
}
#include "input.h"
#include "floresta.h"
#include "simulacao_omp.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo_de_entrada>\n", argv[0]);
        return EXIT_FAILURE;
    }

    Entrada entrada;
    if (!ler_entrada(argv[1], &entrada)) {
        return EXIT_FAILURE;
    }

    //imprimir_entrada(&entrada);

    Floresta floresta;
    if (!criar_floresta(&entrada, &floresta)) {
        liberar_entrada(&entrada);
        return 1;
    }

    //imprimir_floresta(&floresta, entrada.config.linhas, entrada.config.colunas);

    //Usar a entrada para montar a floresta (matriz)

    // Rodar a simulação e calcular os resultados
    ResultadoSimulacao resultado;

    simular_paralelo(&entrada, &floresta, &resultado);

    imprimir_resultados(&resultado);

    liberar_floresta(&floresta);
    liberar_entrada(&entrada);

    return EXIT_SUCCESS;
}
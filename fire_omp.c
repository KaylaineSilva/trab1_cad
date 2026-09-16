#include "input.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo_de_entrada>\n", argv[0]);
        return EXIT_FAILURE;
    }

    Entrada *entrada;
    if (!ler_entrada(argv[1], entrada)) {
        return EXIT_FAILURE;
    }

    imprimir_entrada(entrada);

    liberar_entrada(entrada);
    return EXIT_SUCCESS;
}
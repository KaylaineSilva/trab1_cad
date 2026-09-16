#include "floresta.h"

struct Floresta {
    unsigned char *cobertura;
    unsigned char *umidade;
    
    unsigned char *estado_atual;
    unsigned char *proximo_estado;

    unsigned char *tempo_atual;
    unsigned char *proximo_estado;

    int *ativacao;
}; 
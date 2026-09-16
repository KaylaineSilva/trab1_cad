#include "floresta.h"

struct Celula {
    int cobertura;
    int umidade;
    
    unsigned char estado_atual;
    unsigned char proximo_estado;

    unsigned char tempo_atual;
    unsigned char proximo_tempo;

    int ativacao;
}; 
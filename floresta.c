#include "floresta.h"

struct Celula {
    int cobertura;
    int umidade;
    
    int estado_atual;
    unsigned char proximo_estado;

    int tempo_queima;
    unsigned char proximo_tempo;

    int ativacao;
}; 

int gerar_cobertura(int seed){
    int valor = rand_r(&seed)%100;

    if(valor>=0 && valor<=9){
        return 0;
    } else if (valor>=10 && valor<=19){
        return 1;
    } else if (valor>=20 && valor<=54){
        return 2;
    } else if(valor>=55 && valor<=99){
        return 3;
    }
}

int gerar_umidade(int seed){
    return rand_r(&seed)%101;
}

int gerar_estado_atual(int cobertura){

    //Atribuição com base na cobertura
    switch (cobertura) {
        case 0:
            return 0;
        case 1: 
            return 0;
        case 2:
            return 1;
        case 3: 
            return 1;
    }
}

/*
Saída: 
*/
int criar_floresta(const Entrada *entrada, Celula *celula){
    int l = get_value(entrada, "l");
    int c = get_value(entrada, "c");
    int seed = get_value(entrada, "s");

    //alocar memória para a floresta
    Celula *floresta = malloc((size_t) l * c * sizeof(Celula));

    if(floresta == NULL) {
        fprintf(stderr, "Erro ao alocar ao gerar a matriz da floresta.\n");
        return 0; 
    }

    //gerando cobertura e umidade para cada célula
    for(int i=0; i<l; i++){
        for (int j=0; j<c; j++){

            //calculo do índice
            long long int indice = i*c + j;

            floresta[indice].cobertura = gerar_cobertura(seed);
            floresta[indice].umidade = gerar_umidade(seed);
        }
    }

    for(int i=0; i<l; i++){
        for (int j=0; j<c; j++){

            long long int indice = i*c + j;

            floresta[indice].estado_atual = gerar_estado_atual(floresta[indice].cobertura);
        }
    }
    
    //Aplicando focos iniciais de incêndio
    int qtd_foco = get_value(entrada, 'f');
    Foco *foco; 

    for (int i=0; i<qtd_foco; i++){
        foco = obter_foco(entrada, i);
        int f_linha = get_foco_linha(foco);
        int f_coluna = get_foco_coluna(foco);
        
        long long int indice = f_linha *c+f_coluna;

        //Acessando o elemento da matriz
        //Verificar se é um estado de não combustível -> a entrada é inválida
        if(floresta[indice].estado_atual == 0) {
            fprintf(stderr, "Entrada inválida, um foco de incêndia está sob uma célula não combustível.\n");
            return 0; 
        }

        floresta[indice].estado_atual = 2;

        //Definindo o tempo de queima de acordo com a cobertura
        floresta[indice].tempo_queima = (floresta[indice].cobertura == 2)? 2: 4;

    }
}
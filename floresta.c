#include "floresta.h" 

static void imprimir_matriz(const char *nome, const int *matriz, int linhas, int colunas) {
    printf("=== %s ===\n", nome);

    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            long long indice = (long long) i * colunas + j;

            printf("%4d ", matriz[indice]);
        }

        printf("\n");
    }

    printf("\n");
}

void imprimir_floresta(const Floresta *floresta, int linhas, int colunas) {
    
    printf("Cobertura\n\n");
    imprimir_matriz(
        "COBERTURA",
        floresta->cobertura,
        linhas,
        colunas
    );

    printf("Umidade\n\n");
    imprimir_matriz(
        "UMIDADE",
        floresta->umidade,
        linhas,
        colunas
    );

    printf("Estado Atual\n\n");
    imprimir_matriz(
        "ESTADO ATUAL",
        floresta->estado_atual,
        linhas,
        colunas
    );

    /*imprimir_matriz(
        "PROXIMO ESTADO",
        floresta->proximo_estado,
        linhas,
        colunas
    );*/

    printf("Tempo de queima\n\n");
    imprimir_matriz(
        "TEMPO DE QUEIMA",
        floresta->tempo_atual_queima,
        linhas,
        colunas
    );

    /*imprimir_matriz(
        "PROXIMO TEMPO",
        floresta->proximo_tempo,
        linhas,
        colunas
    );*/

    printf("Ativação\n\n");
    imprimir_matriz(
        "ATIVACAO",
        floresta->ativacao,
        linhas,
        colunas
    );
}

int gerar_cobertura(unsigned int *seed){
    int valor = rand_r(seed)%100;

    if(valor>=0 && valor<=9){
        return 0;
    } else if (valor>=10 && valor<=19){
        return 1;
    } else if (valor>=20 && valor<=54){
        return 2;
    } else if(valor>=55 && valor<=99){
        return 3;
    }

    return -1;
}

int gerar_umidade(unsigned int *seed){
    return rand_r(seed)%101;
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

    return -1; //caso dê erro
}

int alocar_floresta(Floresta *floresta, int linhas, int colunas) {
    size_t total = (size_t) linhas * colunas;

    floresta->cobertura = malloc(total * sizeof *floresta->cobertura);
    floresta->umidade = malloc(total * sizeof *floresta->umidade);

    floresta->estado_atual = malloc(total * sizeof *floresta->estado_atual);
    floresta->proximo_estado = malloc(total * sizeof *floresta->proximo_estado);

    floresta->tempo_atual_queima = malloc(total * sizeof *floresta->tempo_atual_queima);
    floresta->proximo_tempo_queima = malloc(total * sizeof *floresta->proximo_tempo_queima);

    floresta->ativacao = malloc(total * sizeof *floresta->ativacao);

    if (floresta->cobertura == NULL ||
        floresta->umidade == NULL ||
        floresta->estado_atual == NULL ||
        floresta->proximo_estado == NULL ||
        floresta->tempo_atual_queima == NULL ||
        floresta->proximo_tempo_queima == NULL ||
        floresta->ativacao == NULL) {

            
        liberar_floresta(floresta);
        return 0;
    }

    //Preenchendo o vetor com -1
    for (size_t i = 0; i < total; i++) {
        floresta->ativacao[i] = -1;
    }
    return 1;
}

/*
Saída: 
*/
int criar_floresta(const Entrada *entrada, Floresta *floresta){

    int l = entrada->config.linhas;
    int c = entrada->config.colunas;
    unsigned int seed = entrada->config.seed;

    //alocar memória para a floresta
    if(!alocar_floresta(floresta, l, c)) {
        fprintf(stderr, "Erro ao alocar ao gerar a matriz da floresta.\n");
        return 0; 
    }

    //gerando cobertura e umidade para cada célula
    for(long long int i=0; i<l; i++){
        for (long long int j=0; j<c; j++){

            //calculo do índice
            long long int indice = i*c + j;

            floresta->cobertura[indice] = gerar_cobertura(&seed);
            floresta->umidade[indice] = gerar_umidade(&seed);
        }
    }

    for(long long int i=0; i<l; i++){
        for (long long int j=0; j<c; j++){

            long long int indice = i*c + j;

            floresta->estado_atual[indice] = gerar_estado_atual(floresta->cobertura[indice]);
        }
    }
    
    //Aplicando focos iniciais de incêndio
    long long int qtd_foco = entrada->qtd_focos;
    Foco *foco = entrada->focos; 

    for (long long int i=0; i<qtd_foco; i++){
        int f_linha = foco[i].linha;
        int f_coluna = foco[i].coluna;
        
        long long int indice = f_linha *c+f_coluna;

        //Acessando o elemento da matriz
        //Verificar se é um estado de não combustível -> a entrada é inválida
        if(floresta->estado_atual[indice] == 0) {
            fprintf(stderr, "Entrada inválida, um foco de incêndia está sob uma célula não combustível.\n");
            liberar_floresta(floresta);
            return 0; 
        }

        floresta->estado_atual[indice] = 2;

        //Definindo o tempo de queima de acordo com a cobertura
        floresta->tempo_atual_queima[indice] = (floresta->cobertura[indice] == 2)? 2: 4;
    }

    for (long long int z=0; z<entrada->qtd_zonas; z++){
        long long int linha_inicio = entrada->zonas[z].linha_inicio;
        long long int linha_fim = entrada->zonas[z].linha_fim;

        long long int coluna_inicio = entrada->zonas[z].coluna_inicio;
        long long int coluna_fim = entrada->zonas[z].coluna_fim;

        for (long long int i=linha_inicio; i<=linha_fim; i++){
            for (long long int j=coluna_inicio; j<=coluna_fim; j++){
                
                long long int indice = i*c+j;
                
                if(floresta->ativacao[indice] == -1 || floresta->ativacao[indice]>entrada->zonas[z].passo_ativacao){
                    floresta->ativacao[indice] = entrada->zonas[z].passo_ativacao;
                }
            }
        }
    }

    return 1; 
}

void liberar_floresta(Floresta *floresta) {
    if (floresta == NULL) {
        return;
    }

    free(floresta->cobertura);
    free(floresta->umidade);

    free(floresta->estado_atual);
    free(floresta->proximo_estado);

    free(floresta->tempo_atual_queima);
    free(floresta->proximo_tempo_queima);

    free(floresta->ativacao);

    floresta->cobertura = NULL;
    floresta->umidade = NULL;

    floresta->estado_atual = NULL;
    floresta->proximo_estado = NULL;

    floresta->tempo_atual_queima = NULL;
    floresta->proximo_tempo_queima = NULL;

    floresta->ativacao = NULL;
}
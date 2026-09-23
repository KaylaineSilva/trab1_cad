# Simulação Paralela de Incêndio com OpenMP

## Objetivo

Este trabalho foi desenvolvido para a disciplina **SSC0903 — Computação de Alto Desempenho** do ICMC-USP.

O objetivo é implementar uma simulação determinística de propagação de incêndio em uma floresta representada por uma matriz, considerando tipo de cobertura, umidade, vento e zonas de contenção.

Foram desenvolvidas duas versões:

- `fire_seq`: versão sequencial;
- `fire_omp`: versão paralela utilizando OpenMP.

## Como compilar e executar

Para compilar:

```bash
make
```

Para executar a versão sequencial:

```bash
./fire_seq <arquivo_entrada>
```

Para executar a versão paralela:

```bash
./fire_omp <arquivo_entrada>
```

Exemplo:

```bash
./fire_omp entrada_carga_pequena.txt
```

Para executar os testes configurados no Makefile:

```bash
make run-all
```

## Autores

- Roberto Spíndola Abrenhosa Filho
- Julia de Almeida Carvalho
- Kaylaine Bessa da Silva
- Nicolas Carreiro Rodrigues
- Giovanna Lopes de Andrade
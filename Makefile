CC = gcc
CFLAGS = -O2 -Wall -fopenmp

all: fire_seq fire_omp

fire_seq: fire_seq.o input.o floresta.o simulacao_seq.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

fire_omp: fire_omp.o input.o floresta.o simulacao_omp.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

fire_seq.o: fire_seq.c input.h floresta.h simulacao_seq.h
	$(CC) $(CFLAGS) -c $<

fire_omp.o: fire_omp.c input.h floresta.h simulacao_omp.h
	$(CC) $(CFLAGS) -c $<

input.o: input.c input.h
	$(CC) $(CFLAGS) -c $<

floresta.o: floresta.c floresta.h input.h
	$(CC) $(CFLAGS) -c $<

simulacao_seq.o: simulacao_seq.c simulacao_seq.h input.h floresta.h
	$(CC) $(CFLAGS) -c $<

simulacao_omp.o: simulacao_omp.c simulacao_omp.h input.h floresta.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o fire_seq fire_omp

# --- execução -----------------------------------------------------
# Uso:
#   make run                 -> roda com a carga pequena (padrão)
#   make run CARGA=media     -> roda com a carga média
#   make run CARGA=grande    -> roda com a carga grande
#   make run-pequena / run-media / run-grande -> atalhos
#   make run-all             -> roda as três cargas em sequência
#
# Cada chamada compila (se preciso) e roda fire_seq e depois fire_omp,
# um de cada vez, para a mesma entrada.

CARGA ?= pequena

ifeq ($(CARGA),pequena)
ENTRADA = entrada_carga_pequena.txt
else ifeq ($(CARGA),media)
ENTRADA = entrada_carga_media.txt
else ifeq ($(CARGA),grande)
ENTRADA = entrada_carga_grande.txt
else
$(error CARGA inválida: use pequena, media ou grande)
endif

run: all
	@echo "=== fire_seq -- $(ENTRADA) ==="
	./fire_seq $(ENTRADA)
	@echo ""
	@echo "=== fire_omp -- $(ENTRADA) ==="
	./fire_omp $(ENTRADA)

run-pequena:
	$(MAKE) run CARGA=pequena

run-media:
	$(MAKE) run CARGA=media

run-grande:
	$(MAKE) run CARGA=grande

run-all: run-pequena run-media run-grande

.PHONY: all clean run run-pequena run-media run-grande run-all
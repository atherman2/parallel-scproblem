# Layout: source/*.c, headers/*.h, entry points in the project root, binaries in build/
# Usage:
#   make seq / make run_seq file=Testing/Teste_01.dat
#   make para / make run file=Testing/Teste_01.dat   (needs para_main.c)
#   make test [NO_LS=1]
#   make convergence [NO_LS=1]
#   make clean

CC = gcc
CFLAGS = -O2 -Wall -std=gnu11 -Iheaders

SEQ_EXE = build/seq_scproblem$(EXT)
PARA_EXE = build/para_scproblem$(EXT)
EXE_TEST = build/testing$(EXT)
EXE_CONV = build/convergence$(EXT)

DS = source/data_structure.c
RG = source/random_generator.c
LS = source/local_search.c
MUT = source/mutation.c
POP = source/population.c
SEL = source/selection.c
CX = source/crossover.c
GA = source/genetic_algorithm.c
PGA = source/parallel_genetic_algorithm.c
PLS = source/parallel_local_search.c
SRC = $(DS) $(RG) $(LS) $(MUT) $(POP) $(SEL) $(CX) $(GA) $(PGA) $(PLS)
HDR = $(wildcard headers/*.h)

ifeq ($(OS),Windows_NT)
	EXT = .exe
	MKDIR_CMD = if not exist build mkdir build
	CLEAN_CMD = del /q /f build\seq_scproblem.exe build\para_scproblem.exe build\testing.exe build\convergence.exe 2>nul || exit 0
	fix_path = $(subst /,\,$(1))
else
	EXT =
	MKDIR_CMD = mkdir -p build
	CLEAN_CMD = rm -f $(SEQ_EXE) $(PARA_EXE) $(EXE_TEST) $(EXE_CONV)
	fix_path = $(1)
endif

# NO_LS=1 disables the swap local search (test and convergence only)
ifeq ($(NO_LS),1)
	LS_FLAGS = -DNO_LOCAL_SEARCH
else
	LS_FLAGS =
endif

all: $(SEQ_EXE)

build:
	$(MKDIR_CMD)

seq: $(SEQ_EXE)

$(SEQ_EXE): seq_main.c $(SRC) $(HDR) | build
	$(CC) $(CFLAGS) seq_main.c $(SRC) -o $(SEQ_EXE)

run_seq: $(SEQ_EXE)
	$(call fix_path,$(SEQ_EXE)) $(file)

para: $(PARA_EXE)

$(PARA_EXE): para_main.c $(SRC) $(HDR) | build
	$(CC) $(CFLAGS) para_main.c $(SRC) -pthread -o $(PARA_EXE)

run: $(PARA_EXE)
	$(call fix_path,$(PARA_EXE)) $(file)

.PHONY: test
test: | build
	$(CC) $(CFLAGS) $(LS_FLAGS) testing.c $(SRC) -o $(EXE_TEST)
	$(call fix_path,$(EXE_TEST))

.PHONY: convergence
convergence: | build
	$(CC) $(CFLAGS) $(LS_FLAGS) convergence.c $(SRC) -o $(EXE_CONV)
	$(call fix_path,$(EXE_CONV))

.PHONY: all seq para run run_seq clean
clean:
	$(CLEAN_CMD)

SEQ_EXE = build/seq_scproblem
PARA_EXE = build/para_scproblem
EXE_TEST = build/testing
DS = source/data_structure.c
RG = source/random_generator.c
GEN = source/genetic.c

ifeq ($(OS),Windows_NT)
	CLEAN_CMD = del /q /f build\seq_scproblem build\para_scproblem build\testing build\convergence 2>nul || exit 0
else
	CLEAN_CMD = rm -f $(SEQ_EXE) $(PARA_EXE) $(EXE_TEST) build/convergence
endif

all: $(PARA_EXE)

$(PARA_EXE): para_main.c $(DS) $(RG) $(GEN)
	g++ para_main.c $(DS) $(RG) $(GEN) -Iheaders -O2 -o $(PARA_EXE)

run: $(PARA_EXE)
	$(PARA_EXE) $(file)

seq: $(SEQ_EXE)

$(SEQ_EXE): seq_main.c $(DS) $(RG) $(GEN)
	g++ seq_main.c $(DS) $(RG) $(GEN) -Iheaders -O2 -o $(SEQ_EXE)

run_seq: $(SEQ_EXE)
	$(SEQ_EXE) $(file)

ifeq ($(NO_LS),1)
TEST_FLAGS = -DNO_LOCAL_SEARCH
else
TEST_FLAGS =
endif

.PHONY: test
test:
	g++ testing.c $(DS) $(RG) $(GEN) -Iheaders -O2 $(TEST_FLAGS) -o $(EXE_TEST)
	$(EXE_TEST)

ifeq ($(NO_LS),1)
CONV_FLAGS = -DNO_LOCAL_SEARCH
else
CONV_FLAGS =
endif

.PHONY: convergence
convergence:
	g++ convergence.c $(DS) $(RG) $(GEN) -Iheaders -O2 $(CONV_FLAGS) -o build/convergence
	build/convergence

.PHONY: clean
clean:
	$(CLEAN_CMD)

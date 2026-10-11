# Parallel SCProblem

## Compiling

Command | Action
--- | ---
`make` | to compile parallel executable
`make run <file>` | to run the parallel executable with `<file>` argument
`make seq` | to compile sequential executable
`make run_seq <file>` | to run the sequential executable with `<file>` argument

## Files and paths

Nonexistent paths must be created

File/Path | Description
--- | ---
`source/` | `.c` source files
`headers/` | `.h` files
`tests/` | test/input files
`para_main.c` | parallel executable main file
`seq_main.c` | sequential executable main file
`*/data_structure.x` | code related to problem/solution structs
`*/genetic.x` | code related to the genetic algorithm
`*/random.x` | code related to random generation
`headers/util.h` | utility code, included in multiple files

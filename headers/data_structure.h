#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

typedef unsigned int uint;
typedef unsigned char bool_uchar;

struct SCPColumn {
    double cost;
    uint* covered_rows;
    uint num_covered_rows;
    bool_uchar* covers_row;
};
typedef struct SCPColumn SCPColumn;

/* Takes ownership of 'rows'. num_rows is the total number of rows of the instance. */
void create_scp_column(SCPColumn* c, double cost, uint* rows, uint num_rows, uint num_covered_rows);

struct SCPRow {
    uint* covered_by;
    uint num_covered_by;
};
typedef struct SCPRow SCPRow;

/* Takes ownership of 'columns' */
void create_scp_row(SCPRow* r, uint* columns, uint num_covered_by);

struct SCPInstance {
    uint num_rows, num_columns;
    SCPColumn* columns;
    SCPRow* rows;
    uint* columns_range;
    uint* rows_range;
};
typedef struct SCPInstance SCPInstance;

/* Returns 1 on success, 0 on failure (the instance is left empty and safe to pass to free_scp_instance) */
bool_uchar read_scp_instance(SCPInstance* instance, char* file_path);
void free_scp_instance(SCPInstance* instance);
void print_columns(const SCPInstance* instance);
void print_rows(const SCPInstance* instance);

/*
 * Solution arrays have fixed capacity: columns_used holds up to num_columns entries,
 * uncovered_rows up to num_rows entries and column_used exactly num_columns flags.
 * This avoids any reallocation during add/remove operations.
 */
struct SCPSolution {
    double cost;
    uint* columns_used;
    uint* uncovered_rows;
    uint num_columns_used;
    uint num_uncovered_rows;
    bool_uchar* column_used;
};
typedef struct SCPSolution SCPSolution;

/* Takes ownership of columns_used and uncovered_rows (they must have the capacities described above) */
void create_scp_solution(SCPSolution* sol, double cost, uint* columns_used, uint num_columns_used,
                         uint* uncovered_rows, uint num_uncovered_rows, uint num_columns);

/* Allocates a solution with no columns (every row uncovered) */
void scp_solution_empty(SCPSolution* sol, const SCPInstance* instance);
/* Turns an already allocated solution into an empty one, without reallocating */
void scp_solution_reset(SCPSolution* sol, const SCPInstance* instance);
/* Deep copy; dst must already be allocated for the same instance */
void scp_solution_copy(SCPSolution* dst, const SCPSolution* src, const SCPInstance* instance);
void scp_solution_free(SCPSolution* sol);

bool_uchar is_scp_solution_valid(const SCPSolution* sol);
void scp_solution_remove_column(SCPSolution* sol, int column_idx, SCPInstance* instance);
void scp_solution_add_column(SCPSolution* sol, int column_idx, SCPInstance* instance);
void scp_solution_print_columns_used(const SCPSolution* sol);

#endif

#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

typedef unsigned int uint;
typedef unsigned char bool_uchar;

struct SCPColumn {
    double cost;
    uint* covered_rows;
    bool_uchar* covers_row;
};
typedef struct SCPColumn SCPColumn;

void create_scp_column(SCPColumn* c, double cost, uint* rows, int num_rows);

struct SCPRow {
    uint* covered_by;
};
typedef struct SCPRow SCPRow;

void create_scp_row(SCPRow* r, uint* columns);

struct SCPInstance {
    uint num_rows, num_columns;
    SCPColumn* columns;
    SCPRow* rows;
    uint* columns_range, rows_range;
};
typedef struct SCPInstance SCPInstance;

bool_uchar read_scp_instance(SCPInstance* instance, char* file_path);
void print_columns(const SCPInstance* instance);
void print_rows(const SCPInstance* instance);

struct SCPSolution {
    double cost;
    uint* columns_used, uncovered_rows;
    bool_uchar* column_used;
};
void create_scp_solution(SCPSolution* sol, double cost, uint* columns_used, uint* uncovered_rows, int num_columns);
bool_uchar is_scp_solution_valid(const SCPSolution* sol);
void scp_solution_remove_column(SCPSolution* sol, int column_idx, SCPInstance* instance);
void scp_solution_add_column(SCPSolution* sol, int column_idx, SCPInstance* instance);
void scp_solution_print_columns_used(const SCPSolution* sol);

#endif

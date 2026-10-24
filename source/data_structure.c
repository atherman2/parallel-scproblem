#include "data_structure.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

void create_scp_column(SCPColumn *c, double cost, uint *rows, uint num_rows, uint num_covered_rows) {
    c->cost = cost;
    c->covered_rows = rows;
    c->num_covered_rows = num_covered_rows;
    c->covers_row = (bool_uchar*) calloc(num_rows, sizeof(bool_uchar));
    for (uint i = 0; i < num_covered_rows; i++) {
        c->covers_row[rows[i]] = 1;
    }
}

void create_scp_row(SCPRow* r, uint* columns, uint num_covered_by) {
    r->covered_by = columns;
    r->num_covered_by = num_covered_by;
}

/* Reads a full line of any length (without the newline). Returns NULL at EOF. Caller frees. */
static char* read_line(FILE* file) {
    size_t capacity = 256, length = 0;
    char* buffer = (char*) malloc(capacity);
    if (!buffer) return NULL;

    while (fgets(buffer + length, (int)(capacity - length), file)) {
        length += strlen(buffer + length);
        if (length > 0 && buffer[length - 1] == '\n') {
            buffer[--length] = '\0';
            return buffer;
        }
        capacity *= 2;
        char* grown = (char*) realloc(buffer, capacity);
        if (!grown) {
            free(buffer);
            return NULL;
        }
        buffer = grown;
    }

    if (length == 0) {
        free(buffer);
        return NULL;
    }
    return buffer;
}

static int is_blank(const char* text) {
    for (; *text; text++)
        if (!isspace((unsigned char)*text)) return 0;
    return 1;
}

bool_uchar read_scp_instance(SCPInstance* instance, char* file_path) {
    FILE* file;
    char token[32];
    uint num_rows, num_columns;
    uint* row_degree = NULL;
    uint* buffer = NULL;
    size_t buffer_capacity;
    bool_uchar ok = 0;
    int ch;

    memset(instance, 0, sizeof(*instance));

    file = fopen(file_path, "r");
    if (!file) {
        fprintf(stderr, "ERROR: could not open file: %s\n", file_path);
        return 0;
    }

    // Reads the tokens and ignores the text labels, keeping only the numbers
    if (fscanf(file, "%31s %u", token, &num_rows) != 2 ||      // "ROWS"
        fscanf(file, "%31s %u", token, &num_columns) != 2 ||   // "COLUMNS"
        fscanf(file, "%31s", token) != 1) {                    // "DATA"
        fprintf(stderr, "ERROR: invalid header in file: %s\n", file_path);
        fclose(file);
        return 0;
    }
    while ((ch = fgetc(file)) != EOF && ch != '\n');  // discards the rest of the "DATA" line

    instance->num_rows = num_rows;
    instance->num_columns = num_columns;
    instance->columns = (SCPColumn*) calloc(num_columns ? num_columns : 1, sizeof(SCPColumn));
    instance->rows = (SCPRow*) calloc(num_rows ? num_rows : 1, sizeof(SCPRow));
    row_degree = (uint*) calloc(num_rows ? num_rows : 1, sizeof(uint));
    buffer_capacity = num_rows ? num_rows : 1;
    buffer = (uint*) malloc(buffer_capacity * sizeof(uint));

    if (!instance->columns || !instance->rows || !row_degree || !buffer) {
        fprintf(stderr, "ERROR: out of memory reading file: %s\n", file_path);
        goto done;
    }

    // Reading all columns: "<id> <cost> <row> <row> ..." (one per line, rows are 1-based)
    for (uint col = 0; col < num_columns; col++) {
        char* line = NULL;
        while ((line = read_line(file)) != NULL && is_blank(line))
            free(line);

        if (!line) {
            fprintf(stderr, "ERROR: unexpected end of file at column %u in: %s\n", col + 1, file_path);
            goto done;
        }

        char* p = line;
        char* end;
        strtol(p, &end, 10);  // column id (ignored)
        p = end;
        double cost = strtod(p, &end);
        if (end == p) {
            fprintf(stderr, "ERROR: invalid cost at column %u in: %s\n", col + 1, file_path);
            free(line);
            goto done;
        }
        p = end;

        uint count = 0;
        for (;;) {
            long value = strtol(p, &end, 10);
            if (end == p) break;
            p = end;

            if (value < 1 || (unsigned long)value > num_rows) {
                fprintf(stderr, "ERROR: row %ld out of range at column %u in: %s\n", value, col + 1, file_path);
                free(line);
                goto done;
            }
            if (count == buffer_capacity) {
                buffer_capacity *= 2;
                uint* grown = (uint*) realloc(buffer, buffer_capacity * sizeof(uint));
                if (!grown) {
                    fprintf(stderr, "ERROR: out of memory reading file: %s\n", file_path);
                    free(line);
                    goto done;
                }
                buffer = grown;
            }
            buffer[count++] = (uint)(value - 1);
            row_degree[value - 1]++;
        }
        free(line);

        uint* covered_rows = (uint*) malloc((count ? count : 1) * sizeof(uint));
        if (!covered_rows) {
            fprintf(stderr, "ERROR: out of memory reading file: %s\n", file_path);
            goto done;
        }
        memcpy(covered_rows, buffer, count * sizeof(uint));
        create_scp_column(&instance->columns[col], cost, covered_rows, num_rows, count);
    }

    // Builds the row -> columns lists
    for (uint row = 0; row < num_rows; row++) {
        if (row_degree[row] == 0) {
            fprintf(stderr, "ERROR: row %u is not covered by any column (infeasible instance) in: %s\n", row + 1, file_path);
            goto done;
        }
        instance->rows[row].covered_by = (uint*) malloc(row_degree[row] * sizeof(uint));
        if (!instance->rows[row].covered_by) {
            fprintf(stderr, "ERROR: out of memory reading file: %s\n", file_path);
            goto done;
        }
        instance->rows[row].num_covered_by = 0;
    }
    for (uint col = 0; col < num_columns; col++) {
        const SCPColumn* column = &instance->columns[col];
        for (uint i = 0; i < column->num_covered_rows; i++) {
            SCPRow* row = &instance->rows[column->covered_rows[i]];
            row->covered_by[row->num_covered_by++] = col;
        }
    }

    instance->columns_range = (uint*) malloc((num_columns ? num_columns : 1) * sizeof(uint));
    instance->rows_range = (uint*) malloc((num_rows ? num_rows : 1) * sizeof(uint));
    if (!instance->columns_range || !instance->rows_range) {
        fprintf(stderr, "ERROR: out of memory reading file: %s\n", file_path);
        goto done;
    }
    for (uint col = 0; col < num_columns; col++) instance->columns_range[col] = col;
    for (uint row = 0; row < num_rows; row++) instance->rows_range[row] = row;

    ok = 1;

done:
    if (!ok) free_scp_instance(instance);
    free(row_degree);
    free(buffer);
    fclose(file);
    return ok;
}

void free_scp_instance(SCPInstance* instance) {
    if (instance->columns) {
        for (uint i = 0; i < instance->num_columns; i++) {
            free(instance->columns[i].covered_rows);
            free(instance->columns[i].covers_row);
        }
    }
    if (instance->rows) {
        for (uint i = 0; i < instance->num_rows; i++)
            free(instance->rows[i].covered_by);
    }
    free(instance->columns);
    free(instance->rows);
    free(instance->columns_range);
    free(instance->rows_range);
    memset(instance, 0, sizeof(*instance));
}

void print_columns(const SCPInstance* instance) {
    printf("--- COLUMNS ---\n");

    for (uint i = 0; i < instance->num_columns; i++) {
        const SCPColumn* column = &instance->columns[i];
        printf("Column %u | Cost: %g | Covers rows: ", i + 1, column->cost);
        for (uint j = 0; j < column->num_covered_rows; j++)
            printf("%u ", column->covered_rows[j] + 1);
        printf("\n");
    }
}

void print_rows(const SCPInstance* instance) {
    printf("\n--- ROWS ---\n");

    for (uint i = 0; i < instance->num_rows; i++) {
        const SCPRow* row = &instance->rows[i];
        printf("Row %u | Covered by columns: ", i + 1);
        for (uint j = 0; j < row->num_covered_by; j++)
            printf("%u ", row->covered_by[j] + 1);
        printf("\n");
    }
}

void create_scp_solution(SCPSolution* sol, double cost, uint* columns_used, uint num_columns_used,
                         uint* uncovered_rows, uint num_uncovered_rows, uint num_columns) {
    sol->cost = cost;
    sol->columns_used = columns_used;
    sol->num_columns_used = num_columns_used;
    sol->uncovered_rows = uncovered_rows;
    sol->num_uncovered_rows = num_uncovered_rows;
    sol->column_used = (bool_uchar*) calloc(num_columns ? num_columns : 1, sizeof(bool_uchar));
    for (uint i = 0; i < num_columns_used; i++)
        sol->column_used[columns_used[i]] = 1;
}

void scp_solution_empty(SCPSolution* sol, const SCPInstance* instance) {
    sol->columns_used = (uint*) malloc((instance->num_columns ? instance->num_columns : 1) * sizeof(uint));
    sol->uncovered_rows = (uint*) malloc((instance->num_rows ? instance->num_rows : 1) * sizeof(uint));
    sol->column_used = (bool_uchar*) malloc((instance->num_columns ? instance->num_columns : 1) * sizeof(bool_uchar));
    scp_solution_reset(sol, instance);
}

void scp_solution_reset(SCPSolution* sol, const SCPInstance* instance) {
    sol->cost = 0.0;
    sol->num_columns_used = 0;
    sol->num_uncovered_rows = instance->num_rows;
    memcpy(sol->uncovered_rows, instance->rows_range, instance->num_rows * sizeof(uint));
    memset(sol->column_used, 0, instance->num_columns * sizeof(bool_uchar));
}

void scp_solution_copy(SCPSolution* dst, const SCPSolution* src, const SCPInstance* instance) {
    dst->cost = src->cost;
    dst->num_columns_used = src->num_columns_used;
    dst->num_uncovered_rows = src->num_uncovered_rows;
    memcpy(dst->columns_used, src->columns_used, src->num_columns_used * sizeof(uint));
    memcpy(dst->uncovered_rows, src->uncovered_rows, src->num_uncovered_rows * sizeof(uint));
    memcpy(dst->column_used, src->column_used, instance->num_columns * sizeof(bool_uchar));
}

void scp_solution_free(SCPSolution* sol) {
    free(sol->columns_used);
    free(sol->uncovered_rows);
    free(sol->column_used);
    sol->columns_used = NULL;
    sol->uncovered_rows = NULL;
    sol->column_used = NULL;
    sol->num_columns_used = 0;
    sol->num_uncovered_rows = 0;
}

bool_uchar is_scp_solution_valid(const SCPSolution* sol) {
    return sol->num_uncovered_rows == 0;
}

bool_uchar verify_scp_solution(const SCPSolution* sol, const SCPInstance* instance) {
    double sol_cost = 0.0;
    bool_uchar* is_row_covered = (bool_uchar*) calloc(instance->num_rows, sizeof(bool_uchar));
    for (uint i = 0; i < sol->num_columns_used; i++) {
        uint column_idx = sol->columns_used[i];
        SCPColumn* column = &instance->columns[column_idx];
        sol_cost += column->cost;
        for (uint j = 0; j < column->num_covered_rows; j++) {
            uint row_idx = column->covered_rows[j];
            is_row_covered[row_idx] = 1;
        }
    }
    double tolerance = 1e-6 * (1.0 + fabs(sol->cost));
    if (fabs(sol_cost - sol->cost) > tolerance) {
        printf(
            "Solution computed cost is wrong\nSolution verified cost: %f; Solution computed cost: %f\n",
            sol_cost, sol->cost
        );
    }
    for (uint i = 0; i < instance->num_rows; i++) {
        if (!is_row_covered[i]) {
            free(is_row_covered);
            return 0;
        }
    }
    free(is_row_covered);
    return 1;
}

/**
 * Removes a column from the solution.
 * Updates columns_used, column_used and cost, and adds to uncovered_rows
 * every row of the column that no other column in the solution covers.
 */
void scp_solution_remove_column(SCPSolution* sol, int column_idx, SCPInstance* instance) {
	// Removes column_idx from columns_used (order is not preserved: last element takes its place)
	for (uint i = 0; i < sol->num_columns_used; i++) {
		if ((int)sol->columns_used[i] == column_idx) {
			sol->columns_used[i] = sol->columns_used[sol->num_columns_used - 1];
			sol->num_columns_used--;
			break;
		}
	}
	sol->column_used[column_idx] = 0;

	SCPColumn* column = &instance->columns[column_idx];
	sol->cost -= column->cost;

	for (uint i = 0; i < column->num_covered_rows; i++) {
		uint row_idx = column->covered_rows[i];
		SCPRow* row = &instance->rows[row_idx];

		bool_uchar still_covered = 0;
		for (uint j = 0; j < row->num_covered_by; j++) {
			uint other_column_idx = row->covered_by[j];
			if ((int)other_column_idx != column_idx && sol->column_used[other_column_idx]) {
				still_covered = 1;
				break;
			}
		}

		if (!still_covered)
			sol->uncovered_rows[sol->num_uncovered_rows++] = row_idx;
	}
}

/**
 * Adds a column to the solution.
 * Updates columns_used, column_used and cost, and removes from uncovered_rows
 * the rows that the column covers.
 */
void scp_solution_add_column(SCPSolution* sol, int column_idx, SCPInstance* instance) {
	sol->columns_used[sol->num_columns_used++] = (uint)column_idx;
	sol->column_used[column_idx] = 1;

	SCPColumn* column = &instance->columns[column_idx];
	sol->cost += column->cost;

	for (uint i = 0; i < column->num_covered_rows; i++) {
		uint row_idx = column->covered_rows[i];

		// Searches the row in uncovered_rows and removes it (last element takes its place)
		for (uint j = 0; j < sol->num_uncovered_rows; j++) {
			if (sol->uncovered_rows[j] == row_idx) {
				sol->uncovered_rows[j] = sol->uncovered_rows[sol->num_uncovered_rows - 1];
				sol->num_uncovered_rows--;
				break;
			}
		}
	}
}

static int compare_uint(const void* a, const void* b) {
    uint x = *(const uint*)a, y = *(const uint*)b;
    return (x > y) - (x < y);
}

void scp_solution_print_columns_used(const SCPSolution* sol) {
    // Sorts a copy so the solution itself is not modified
    uint* sorted = (uint*) malloc((sol->num_columns_used ? sol->num_columns_used : 1) * sizeof(uint));
    memcpy(sorted, sol->columns_used, sol->num_columns_used * sizeof(uint));
    qsort(sorted, sol->num_columns_used, sizeof(uint), compare_uint);

    for (uint i = 0; i < sol->num_columns_used; i++)
        printf("%u, ", sorted[i] + 1);
    printf("\n");

    free(sorted);
}

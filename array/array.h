
#ifndef _ARRAY_H_
#define _ARRAY_H_

#include "../mempool/mempool.h"

typedef enum _array_axis {
    array_axis_x,
    array_axis_0,
    array_axis_1,
} array_axis;

typedef struct _array {
	int rows;
	int cols;
	double* x;
    //double* T;
} array;

typedef struct _array_index {
    size_t size;
    int* indexes;
} array_index;

typedef struct _array_instance {
	mempool_instance *mem;
} array_instance;

extern void array_init(array_instance* self, mempool_instance* mem);
extern void array_set(array* a, int rows, int cols, double n);
extern double array_get(array* a, int rows, int cols);
extern void array_set_cols(array* a, int rows, array* b);

extern array* array_create(array_instance* self, int rows, int cols);
extern array* array_zeros(array_instance* self, int rows, int cols);
extern array* array_from(array_instance* self, double* src, int rows, int cols);
extern void array_dump(array* a);
extern array* array_load_text(array_instance* self, const char* filename);

extern array* array_transpose(array_instance* self, array* a);
extern array* array_select_cols(array_instance* self, array* a, array_index* ai);
extern array* array_select_rows(array_instance* self, array* a, array_index* ai);
extern array* array_row_reference(array_instance* self, array* a, int rowindex);

//########################################
//# broad cast
//########################################
extern array* array_broadcast_subtract(array_instance* self, array* a, array* b);

//########################################
//# array calculation
//########################################
extern array* array_sum(array_instance* self, array* a, array_axis axis);
extern array* array_mean(array_instance* self, array* a, array_axis axis);
extern array* array_pow(array_instance* self, array* a, double y, array_axis axis);
extern array* array_sqrt(array_instance* self, array* a, array_axis axis);
extern array* array_norm(array_instance* self, array* a, array_axis axis);

//########################################
//# search index
//########################################
extern array_index* array_arg_min(array_instance* self, array* a, array_axis axis);
extern array_index* array_where_eq(array_instance* self, array* a, int index, double n);

//########################################
//# index operation
//########################################
extern array_index* array_index_create(array_instance* self, size_t size);
extern array_index* array_index_from(array_instance* self, int* indexes, size_t size);
extern void array_index_dump(array_index* ai);

#endif

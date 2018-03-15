
#include <float.h>
#include <math.h>
#include <memory.h>
#include <omp.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "./array.h"
#include "../exception/exception.h"
#include "../format/format.h"
#include "../mempool/mempool.h"
#include "../mylog/mylog.h"

//#pragma GCC target ("avx") // ターゲットの変更 sse4, avx, avx2 など
#pragma GCC target ("sse4")

void array_init(array_instance* self, mempool_instance* mem) {
	self->mem = mem;
}

size_t array_memory_size(int rows, int cols) {
	return sizeof(double) * rows * cols;
}

int array_get_index(int cols, int row, int col) {
    return cols * row + col;
}

double array_get(array* a, int rows, int cols) {
    return a->x[array_get_index(a->cols, rows, cols)];
}

void array_set(array* a, int rows, int cols, double n) {
    a->x[array_get_index(a->cols, rows, cols)] = n;
}

void array_set_cols(array* a, int rows, array* b) {
    //printf("%s, %d\n", __FUNCTION__, __LINE__);
    if(a->cols != b->cols) {
        exception_jump(format("error %s, column size is different %d, %d", __FUNCTION__, a->cols, b->cols), 9);
    }
    memcpy(&a->x[rows * a->cols], b->x, sizeof(double) * a->cols);
}

double array_1d_get(array* a, int rows) {
    return a->x[rows];
}

void array_1d_set(array* a, int rows, double n) {
    a->x[rows] = n;
}

void array_fill(array_instance* self, array* a, double n) {
	int i;
	int j;
	for(i = 0; i < a->rows; i++) {
		for(j = 0; j < a->cols; j++) {
			a->x[i * a->cols + j] = n;
		}
	}
}

array* array_create(array_instance* self, int rows, int cols) {
	char* m = mempool_alloc(self->mem, sizeof(array) + array_memory_size(rows, cols));
	array* a = (array*)m;
	a->rows = rows;
	a->cols = cols;
	a->x = (double*)(m + sizeof(array));
	return a;
}

array* array_zeros(array_instance* self, int rows, int cols) {
	array* a = array_create(self, rows, cols);
	array_fill(self, a, 0);
	return a;
}

array* array_from(array_instance* self, double* src, int rows, int cols) {
	array* a = array_create(self, rows, cols);
	int i;
	int j;
	for(i = 0; i < a->rows; i++) {
		for(j = 0; j < a->cols; j++) {
			array_set(a, i, j, src[array_get_index(a->cols, i, j)]);
		}
	}
	return a;
}

void array_dump(array* a) {
	int i;
	int j;
	for(i = 0; i < a->rows; i++) {
		for(j = 0; j < a->cols; j++) {
			if(j != 0) {
				printf("\t");
			}
			printf("%lf", array_get(a, i, j));
		}
		puts("");
	}
}

size_t file_get_size(const char* filename) {
	struct stat buffer;
    if (stat(filename, &buffer) != 0) {
		exception_jump(format("error %s, stat failed, %s", __FUNCTION__, filename), 999);
	}
	return buffer.st_size;
 }

char* text_malloc_read(const char* filename) {
	size_t filesize = file_get_size(filename);
	char *text = malloc(filesize + 1);
	FILE* fp = fopen(filename, "rb");
	if(fp == NULL) {
		exception_jump(format("error %s, fopen failed, %s", __FUNCTION__, filename), 999);
	}
	fread(text, sizeof(char), filesize, fp);
	text[filesize] = '\0';
	return text;
}

array* array_load_text(array_instance* self, const char* filename) {
	char* text = text_malloc_read(filename);
	const char* p = text;
	int tab = 0;
	int cr = 0;
	int lf = 0;
	int rows = 0;

	// rows, cols check
	while(*p) {
		if(*p == '\t') {
			tab++;
		}
		if(*p == '\r') {
			cr++;
		}
		if(*p == '\n') {
			lf++;
		}
		p++;
	}
	if(cr == 0 || lf == 0) {
		if(cr > lf) {
			rows = cr;
		} else {
			rows = lf;
		}
	}
	tab += rows;

	if((tab % rows) != 0) {
		exception_jump(format("error %s, illegal matrix size, total=%d, rows=%d", __FUNCTION__, tab, rows), 999);
	}

	// check end
	// read tsv
	int cols = tab / rows;
	array* a = array_create(self, rows, cols);
	p = text;
	int index = 0;
	while(*p) {
		a->x[index] = atof(p);
		while(*p != '\t' && *p != '\r' && *p != '\n' && *p != '\0') {
			p++;
		}
		if(*p == '\0') {
			break;
		}
		while(*p == '\t' || *p == '\r' || *p == '\n') {
			p++;
		}
		index++;
	}
	free(text);
	return a;
}

//########################################
//# transpose
//########################################
array* array_transpose(array_instance* self, array* a) {
    array* T;
    T = array_create(self, a->cols, a->rows);
    int i;
    int j;
    for(i = 0; i < a->rows; i++) {
        for(j = 0; j < a->cols; j++) {
            T->x[j * T->cols + i] = a->x[i * a->cols + j];
        }
    }
    return T;
}

//########################################
//# select
//########################################
array* array_select_cols(array_instance* self, array* a, array_index* ai) {
	int i;
	int j;
	printf("%s, %d", __FILE__, __LINE__);
	array* res = array_create(self, a->rows, ai->size);
	for(i = 0; i < a->rows; i++) {
		for(j = 0; j < ai->size; j++) {
			//array_set(res, i, j, array_get(a, i, colindex[j]));
            array_set(res, i, j, array_get(a, i, ai->indexes[j]));
		}
	}
	return res;
}

array* array_select_rows(array_instance* self, array* a, array_index* ai) {
	int i;
    int length = ai->size;
	array* res = array_create(self, length, a->cols);
	for(i = 0; i < length; i++) {
        /*
        printf("%s, %d\n", __FUNCTION__, __LINE__);
        printf("i * res->cols = %d\n", i * res->cols);
        printf("sizeof(double) * a->cols = %d\n", sizeof(double) * a->cols);
        */
		memcpy(&res->x[i * res->cols], &a->x[ai->indexes[i] * a->cols], sizeof(double) * a->cols);

        /*
        for(j = 0; j < a->cols; j++) {
            array_set(res, i, j, array_get(a, ai->indexes[i], j));
        }
        */
	}
	return res;
}

array* array_row_reference(array_instance* self, array* a, int rowindex) {
	char* m = mempool_alloc(self->mem, sizeof(array));
	array* res = (array*)m;
	res->rows = 1;
	res->cols = a->cols;
	//array_get_index(int cols, int row, int col) {
	res->x = &a->x[array_get_index(a->cols, rowindex, 0)];
	return res;
}

//########################################
//# broad cast
//########################################
void array_broadcast_check(array* a, array* b) {
	if(b->rows != 1) {
		exception_jump(format("error %s, right side is  must be 1 rows, rows=%d", __FUNCTION__, b->rows), 9);
	}
	if(a->cols != b->cols) {
		exception_jump(format("error %s, columns must be same, cols %d != %d", __FUNCTION__, a->cols, b->cols), 9);
	}
}
array* array_broadcast_subtract(array_instance* self, array* a, array* b) {
	array_broadcast_check(a, b);
	int i;
	int j;
	array* result = array_create(self, a->rows, a->cols);
	for(i = 0; i < a->rows; i++) {
		for(j = 0; j < a->cols; j++) {
			//result->x[array_get_index(a->cols, i, j)] = array_get(a, i, j) - array_get(b, 0, j)
			array_set(result, i, j, array_get(a, i, j) - array_get(b, 0, j));
		}
	}
	return result;
}

//########################################
//# array calculation
//########################################
double array_function_sum(double* a, int size, int skip) {
    double res = 0;
    while(size) {
        size--;
        res += a[size * skip];
    }
    return res;
}

double array_function_mean(double* a, int size, int skip) {
    double res = array_function_sum(a, size, skip);
    return res / size;
}

array* array_sum(array_instance* self, array* a, array_axis axis) {
    array* result;
    int i;
    double sum;
    if(axis == array_axis_0) {
        result = array_create(self, 1, a->cols);
        for(i = 0; i < a->cols; i++) {
            sum = array_function_sum(&a->x[i], a->rows, a->cols);
            array_set(result, 0, i, sum);
        }
        return result;
    } else if(axis == array_axis_1) {
        result = array_create(self, a->rows, 1);
        for(i = 0; i < a->rows; i++) {
            sum = array_function_sum(&a->x[i * a->cols], a->cols, 1);
            array_set(result, i, 0, sum);
        }
        return result;
    } else {
        //cols = 1;
        return NULL;
    }
}

array* array_mean(array_instance* self, array* a, array_axis axis) {
    array* result;
    int i;
    double mean;
    if(axis == array_axis_0) {
        result = array_create(self, 1, a->cols);
        for(i = 0; i < a->cols; i++) {
            mean = array_function_mean(&a->x[i], a->rows, a->cols);
            array_set(result, 0, i, mean);
        }
        return result;
    } else if(axis == array_axis_1) {
        result = array_create(self, a->rows, 1);
        for(i = 0; i < a->rows; i++) {
            mean = array_function_mean(&a->x[i * a->cols], a->cols, 1);
            array_set(result, i, 0, mean);
        }
        return result;
    } else {
        //cols = 1;
        return NULL;
    }
}

array* array_pow(array_instance* self, array* a, double y, array_axis axis) {
    array* res = array_create(self, a->rows, a->cols);
    int i;
    int j;
    int idx;
    //#pragma omp parallel for
    for(i = 0; i < a->rows; i++) {
        for(j = 0; j < a->cols; j++) {
            idx = i * a->cols + j;
            res->x[idx] = powl(a->x[idx], y);
        }
    }

    return res;
}

array* array_sqrt(array_instance* self, array* a, array_axis axis) {
    array* res = array_create(self, a->rows, a->cols);
    int i;
    int j;
    int idx;
    for(i = 0; i < a->rows; i++) {
        for(j = 0; j < a->cols; j++) {
            idx = i * a->cols + j;
            res->x[idx] = sqrtl(a->x[idx]);
        }
    }

    return res;
}

array* array_norm(array_instance* self, array* a, array_axis axis) {
    int i;
    int j;
	double sum;
	array* result;
    if(axis == array_axis_0) {
		result = array_create(self, 1, a->cols);
		for(i = 0; i < a->cols; i++) {
			sum = 0;
			for(j = 0; j < a->rows; j++) {
				sum += powl(array_get(a, i, j), 2);
			}
			array_set(result, 0, i, sqrtl(sum));
		}
		return result;
	} else if(axis == array_axis_1) {
		result = array_create(self, a->rows, 1);
		for(i = 0; i < a->rows; i++) {
			sum = 0;
			for(j = 0; j < a->cols; j++) {
				sum += powl(array_get(a, i, j), 2);
			}
			array_set(result, i, 0, sqrtl(sum));
		}
		return result;
	} else {
		exception_jump(format("erro %s, illegal axis %d", __FUNCTION__, axis), 9);
		return NULL;
	}
}

//########################################
//# search index
//########################################
/*
FLT_MIN	float型の浮動小数点で表現できる最小値
FLT_MAX	float型の最大値
DBL_MIN	double型の浮動小数点で表現できる最小値
DBL_MAX	double型の最大値
LDBL_MIN	long double の浮動小数点で表現できる最小値
LDBL_MAX	long double の最大値
*/
array_index* array_arg_min(array_instance* self, array* a, array_axis axis) {
	array_index* result;
	int i;
	int j;
	double max;
	double n;
	if(axis == array_axis_0) {
		result = array_index_create(self, a->cols);
		for(i = 0; i < a->cols; i++) {
			max = DBL_MAX;
			for(j = 0; j < a->rows; j++) {
				n = array_get(a, j, i);
				if(n < max) {
					max = n;
					result->indexes[i] = j;
				}
			}
		}
		return result;
	} else if(axis == array_axis_1) {
		result = array_index_create(self, a->rows);
		for(i = 0; i < a->rows; i++) {
			max = DBL_MAX;
			for(j = 0; j < a->cols; j++) {
				n = array_get(a, i, j);
				if(n < max) {
					max = n;
					result->indexes[i] = j;
				}
			}
		}
		return result;
	} else {
		exception_jump(format("erro %s, illegal axis %d", __FUNCTION__, axis), 9);
		return NULL;
	}
}


//########################################
//# index operation
//########################################
array_index* array_where_eq(array_instance* self, array* a, int index, double n) {
    int i;
    int count = 0;
    array_index* result = array_index_create(self, a->rows);
    for(i = 0; i < a->rows; i++) {
        if(array_get(a, i, index) == n) {
            result->indexes[count] = i;
            count++;
        }
    }
    result->size = count;
    return result;
}

array_index* array_index_create(array_instance* self, size_t size) {
    char* m = mempool_alloc(self->mem, sizeof(array_index) + sizeof(int) * size);
    array_index* ai = (array_index*)m;
    ai->size = size;
    ai->indexes = (int*)(m + sizeof(array_index));
    return ai;
}

array_index* array_index_from(array_instance* self, int* indexes, size_t size) {
    array_index* ai = array_index_create(self, size);
    int i;
    for(i = 0; i < size; i++) {
        ai->indexes[i] = indexes[i];
    }
    return ai;
}

void array_index_dump(array_index* ai) {
	int i;
	for(i = 0; i < ai->size; i++) {
        printf("%d", ai->indexes[i]);
        puts("");
	}
}

#ifdef ARRAY_UNITTEST

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof(x[0]))

#include <string.h>

int main(int argc, char** argv) {
	exception_begin();
	mylog_init(NULL, NULL, NULL);
	mylog_info(format("main start, build: %s %s", __DATE__, __TIME__));
	mempool_instance mem;
	mempool_init(&mem, 1024 * 1024 * 1024);
	array_instance instance;
	array_init(&instance, &mem);

	array* a1 = array_zeros(&instance, 3, 4);
	array_dump(a1);
	array_fill(&instance, a1, 999.444);
	array_dump(a1);
	array* a2 = array_load_text(&instance, "./seeds_dataset.txt");
	array_dump(a2);

    int xindexesI[] = {0, 1, 2};
    array_index* xindexes = array_index_from(&instance, xindexesI, ARRAY_LENGTH(xindexesI));
    array* x = array_select_cols(&instance, a2, xindexes);
    array_dump(x);

    array* sum_col = array_sum(&instance, x, array_axis_0);
    array_dump(sum_col);
    array* sum_row = array_sum(&instance, x, array_axis_1);
    array_dump(sum_row);

    array* mean_col = array_mean(&instance, x, array_axis_0);
    array_dump(mean_col);
    array* mean_row = array_mean(&instance, x, array_axis_1);
    array_dump(mean_row);

	mempool_end(&mem);
	return 0;
}
//./array > result.txt

#endif

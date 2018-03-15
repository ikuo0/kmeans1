
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../exception/exception.h"
#include "../format/format.h"
#include "../mylog/mylog.h"

#define XINDEX(DIM, ROWS, COLS) (ROWS * DIM + COLS)

double random_double(double N) {
    return ((double)rand() / ((double)RAND_MAX + 1)) * N;
}

typedef struct _iarray {
    size_t size;
    int* a;
} iarray;

typedef struct _darray {
    size_t size;
    double* a;
} darray;

iarray* malloc_int_array(size_t size) {
    char* m = malloc(sizeof(iarray) + sizeof(int) * size);
    iarray* result = (iarray*)m;
    result->size = size;
    result->a = (int*)(m + sizeof(iarray));
    return result;
}

darray* malloc_double_array(size_t size) {
    char* m = malloc(sizeof(darray) + sizeof(double) * size);
    darray* result = (darray*)m;
    result->size = size;
    result->a = (double*)(m + sizeof(darray));
    return result;
}

typedef struct _kmeans_params {
    int n_clusters;
    int max_iter;
    double tol;
    size_t length;
    size_t dim;
} kmeans_params;

typedef struct _kmeans_work_variables {
	iarray* indexes;
	darray* mean;
	darray* cluster_centers_;
	iarray* labels_;
	darray* difference;
	darray* distances;
	darray* centroid_moves;
} kmeans_work_variables;

//########################################
//# file operation
//########################################
size_t file_get_size(const char* filename) {
	struct stat buffer;
    if (stat(filename, &buffer) != 0) {
		exception_jump(format("error %s, stat failed, %s", __FUNCTION__, filename), 9);
	}
	return buffer.st_size;
 }

char* malloc_and_read_text(const char* filename) {
	size_t filesize = file_get_size(filename);
	char *text = malloc(filesize + 1);
	FILE* fp = fopen(filename, "rb");
	if(fp == NULL) {
		exception_jump(format("error %s, fopen failed, %s", __FUNCTION__, filename), 9);
	}
	fread(text, sizeof(char), filesize, fp);
	text[filesize] = '\0';
	return text;
}

void get_tsv_size(const char* filename, int *rowcol) {
    char* text = malloc_and_read_text(filename);
    const char* p = text;

	int tab = 0;
	int cr = 0;
	int lf = 0;
    int rows;
    int cols;
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

    if((cr != 0 && lf == 0) || (cr == lf)) {
        rows = cr;
    } else if(cr == 0 && lf != 0) {
        rows = lf;
    } else if(cr != lf) {
		rows = 0;
        exception_jump(format("error in %s, cr and lf is not same, cr=%d, lf=%d", __FUNCTION__, cr, lf), 9);
    }

    tab += rows;

	if((tab % rows) != 0) {
		exception_jump(format("error in %s, illegal matrix size, total=%d, rows=%d", __FUNCTION__, tab, rows), 9);
	}

    cols = tab / rows;

    rowcol[0] = rows;
    rowcol[1] = cols;
    free(text);
}

darray* malloc_and_read_tsv(const char* filename, int* rec_tsvinfo) {
    int tsvinfo[2];// [0] = rows, [1] = cols
    get_tsv_size(filename, tsvinfo);
    mylog_info(format("%s rows=%d, cols=%d", filename, tsvinfo[0], tsvinfo[1]));
    darray* a = malloc_double_array(tsvinfo[0] * tsvinfo[1]);
    char* text = malloc_and_read_text(filename);
    char* p = text;

	p = text;
	int index = 0;
	while(*p) {
		a->a[index] = atof(p);
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

    if(rec_tsvinfo != NULL) {
        memcpy(rec_tsvinfo, tsvinfo, sizeof(tsvinfo));
    }

    free(text);

    return a;
}

//########################################
//# int array operations
//########################################
void iarray_dump(iarray* a, int rows, int cols) {
    int i;
    int j;
    for(i = 0; i < rows; i++) {
        for(j = 0; j < cols; j++) {
            if(j != 0) {
                printf("\t");
            }
            printf("%d", a->a[i * cols + j]);
        }
        puts("");
    }
}

int iarray_where_eq(iarray* a, int n, iarray* dst) {
    int i;
    int j;
    for(i = 0, j = 0; i < a->size; i++) {
        if(a->a[i] == n) {
            dst->a[j] = i;
            j++;
        }
    }
	dst->size = j;
    return j;
}

//########################################
//# double array operations
//########################################
void darray_dump(darray* a, int rows, int cols) {
    int i;
    int j;
    for(i = 0; i < rows; i++) {
        for(j = 0; j < cols; j++) {
            if(j != 0) {
                printf("\t");
            }
            printf("%lf", a->a[i * cols + j]);
        }
        puts("");
    }
}

void darray_fill(darray* a, double n) {
	int i;
	for(i = 0; i < a->size; i++) {
		a->a[i] = n;
	}
}

void array_mean(darray* x, int dim, darray* dst, iarray* indexes) {
    int i;
    int j;
    double sum;
    for(i = 0; i < dim; i++) {
        sum = 0;
        for(j = 0; j < indexes->size; j++) {
            sum += x->a[XINDEX(dim, indexes->a[j], i)];
        }
        dst->a[i] = sum / (double)indexes->size;
    }
}

double array_norm(darray* a, int dim) {
	int i;
	double sum = 0;
	for(i = 0; i < dim; i++) {
		sum += powl(a->a[i], 2);
	}
	return sqrtf(sum);
}

void kmeans_initial_random(kmeans_params* self, iarray* labels_) {
    int i;
    for(i = 0; i < self->length; i++) {
        labels_->a[i] = i % self->n_clusters;
    }
}


void array_subtract(double* a, double* b, int dim, darray* dst) {
	int i;
	for(i = 0; i < dim; i++) {
		dst->a[i] = a[i] - b[i];
	}
}

int array_arg_min(darray* a) {
	int i;
	double min = DBL_MAX;
	int index = 0;
	for(i = 0; i < a->size; i++) {
		if(a->a[i] < min) {
			min = a->a[i];
			index = i;
		}
	}
	return index;
}

double array_max(darray* a) {
	int i;
	int max = DBL_MIN;
	for(i  =0; i < a->size; i++) {
		if(a->a[i] > max) {
			max = a->a[i];
		}
	}
	return max;
}

void kmeans_calculate_centers(kmeans_params* params, kmeans_work_variables* vars, darray* x) {
    int n_clusters = params->n_clusters;
    //int max_iter = params->max_iter;
    //double tol = params->tol;
    //size_t length = params->length;
    size_t dim = params->dim;
	iarray* indexes = vars->indexes;
	darray* mean = vars->mean;
	darray* cluster_centers_ = vars->cluster_centers_;
	iarray* labels_ = vars->labels_;
	darray* difference = vars->difference;
	darray* centroid_moves = vars->centroid_moves;
	int i;

	for(i = 0; i < n_clusters; i++) {
		iarray_where_eq(labels_, i, indexes);
		array_mean(x, dim, mean, indexes);
		//difference
		//tol = array_norm(mean, dim);
		array_subtract(&cluster_centers_->a[XINDEX(dim, i, 0)], mean->a, dim, difference);
		centroid_moves->a[i] = array_norm(difference, dim);
		memcpy(&cluster_centers_->a[XINDEX(dim, i, 0)], mean->a, sizeof(double) * mean->size);
	}
}

void kmeans_predict(kmeans_params* params, kmeans_work_variables* vars, darray* x) {
    int n_clusters = params->n_clusters;
    size_t length = params->length;
    size_t dim = params->dim;
	darray* cluster_centers_ = vars->cluster_centers_;
	iarray* labels_ = vars->labels_;
	darray* difference = vars->difference;
	darray* distances = vars->distances;
	int i;
	int ki;
	for(i = 0; i < length; i++) {
		for(ki = 0; ki < n_clusters; ki++) {
			array_subtract(&cluster_centers_->a[XINDEX(dim, ki, 0)], &x->a[XINDEX(dim, i, 0)], dim, difference);
			distances->a[ki] = array_norm(difference, dim);
		}
		labels_->a[i] = array_arg_min(distances);
	}
}

void kmeans_train(kmeans_params* params, kmeans_work_variables* vars, darray* x) {
	int max_iter = params->max_iter;
	double tol = params->tol;
	iarray* labels_ = vars->labels_;
	darray* centroid_moves = vars->centroid_moves;
    int i;

    // 初期値
    kmeans_initial_random(params, labels_);

	for(i = 0; i < max_iter; i++) {
		// 中心計算
		kmeans_calculate_centers(params, vars, x);

		// ラベリング
		kmeans_predict(params, vars, x);

		if(array_max(centroid_moves) <= tol) {
			break;
		}
	}
}

void kmeans_init(kmeans_params* params, kmeans_work_variables* vars, int n_clusters, int length, int dim) {
	// parameters
    params->n_clusters = n_clusters;
    params->max_iter = 10;
    params->tol = 1e-4;
	params->length = length;
	params->dim = dim;

	// variables
	vars->indexes = malloc_int_array(length);
	vars->mean = malloc_double_array(dim);
	vars->cluster_centers_ = malloc_double_array(params->n_clusters * dim);
	vars->labels_ = malloc_int_array(length);
	vars->difference = malloc_double_array(dim);
	vars->distances = malloc_double_array(n_clusters);
	vars->centroid_moves = malloc_double_array(n_clusters);
	darray_fill(vars->centroid_moves, DBL_MAX);
}

void kmeans_end(kmeans_work_variables* vars) {
    free((char*)vars->indexes);
    free((char*)vars->mean);
	free((char*)vars->cluster_centers_);
	free((char*)vars->labels_);
	free((char*)vars->difference);
	free((char*)vars->distances);
	free((char*)vars->centroid_moves);
}

int main(int argc, char** argv) {
    exception_begin();
    mylog_init(NULL, NULL, NULL);
    mylog_info(format("main start, %s %s", __DATE__, __TIME__));

    // 行列の準備
    int tsvinfo[2];
    darray* x = malloc_and_read_tsv("./x.tsv", tsvinfo);
	darray_dump(x, tsvinfo[0], tsvinfo[1]);
	darray* y = malloc_and_read_tsv("./y.tsv", NULL);

    //
	int n_clusters = 3;
    kmeans_params params;
	kmeans_work_variables vars;
	kmeans_init(&params, &vars, n_clusters, tsvinfo[0], tsvinfo[1]);

	mylog_info(format("start kmeans train"));
	kmeans_train(&params, &vars, x);
	mylog_info(format("end kmeans train"));

	puts("predicts");
	iarray_dump(vars.labels_, params.length, 1);
	puts("answers");
	darray_dump(y, params.length, 1);

	kmeans_end(&vars);

    return 0;
}

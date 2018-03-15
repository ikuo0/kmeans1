
#include <stdlib.h>

#include "../array/array.h"
#include "../exception/exception.h"
#include "../format/format.h"
#include "../mempool/mempool.h"
#include "../mylog/mylog.h"

double random_double(double N) {
    return ((double)rand() / ((double)RAND_MAX + 1)) * N;
}

array* loaddata(array_instance *instance) {
    int rows = 4000;
    int cols = 25;
    array* a = array_create(instance, rows, cols);

    int i;
    int j;
    for(i = 0; i < rows; i++) {
        for(j = 0; j < cols; j++) {
            array_set(a, i, j, random_double(1));
        }
    }
    return a;
}

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof(x[0]))

typedef struct _kmeans_instance {
    array_instance* arr;
    int k;
    int dim;
    array* means;

    int iterations;
    double threshold;
} kmeans_instance;

void kmeans_init(kmeans_instance* self, array_instance* arr, int k, int dim) {
    self->arr = arr;
    self->k = k;
    self->dim = dim;
    self->means = array_zeros(self->arr, k, dim);

    // parameters
    self->iterations = 300;
    self->threshold = 1e-4;
}

array* kmeans_calculate_centers(kmeans_instance* self, array* predicts, array* x) {
    //printf("%s, %d\n", __FUNCTION__, __LINE__);
	mempool_instance *mem = self->arr->mem;
    array_instance* arr = self->arr;
    array_index* indexes;
    int k = self->k;
    int dim = self->dim;
    int ki;
    array* xi;
    array* means = array_create(arr, k, dim);
    array* mean;
    for(ki = 0; ki < k; ki++) {
        indexes = array_where_eq(arr, predicts, 0, (double)ki);
        xi = array_select_rows(arr, x, indexes);
        mean = array_mean(arr, xi, array_axis_0);
        array_set_cols(means, ki, mean);

        // free
		mempool_free(mem, (char*)indexes);
        mempool_free(mem, (char*)xi);
        mempool_free(mem, (char*)mean);
    }
    return means;
}

array* kmeans_predict(kmeans_instance* self, array* means, array* x) {
	array_instance* arr = self->arr;
	mempool_instance *mem = self->arr->mem;
    int i;
    size_t xlen = x->rows;
    array* xi;
    array* diff;
    array* norm;
	array* predicts = array_create(arr, xlen, 1);
	array_index* min_index;
    for(i = 0; i < xlen; i++) {
        xi = array_row_reference(arr, x, i);
        diff = array_broadcast_subtract(arr, means, xi);
        norm = array_norm(arr, diff, array_axis_1);
		min_index = array_arg_min(arr, norm, array_axis_0);
		array_set(predicts, i, 0, min_index->indexes[0]);

		// free
		mempool_free(mem, (char*)xi);
		mempool_free(mem, (char*)diff);
		mempool_free(mem, (char*)norm);
		mempool_free(mem, (char*)min_index);
    }
    return predicts;
}

array* kmeans_train(kmeans_instance* self, array* x) {
    int k = self->k;
    int iterations = self->iterations;
    int xlen = x->rows;
    array_instance* arr = self->arr;
	mempool_instance *mem = self->arr->mem;
    array* predicts = array_zeros(arr, xlen, 1);
    int i;
    int ki;
    // set random cluster
    for(i = 0, ki = 0; i < xlen; i++, ki++) {
        array_set(predicts, i, 0, (double)(ki % k));
    }
	/*
    array_dump(predicts);

    array_index* indexes = array_where_eq(arr, predicts, 0, (double)0);
    array_index_dump(indexes);

    array* xi = array_select_rows(arr, x, indexes);
    array_dump(xi);

    array* means = kmeans_calculate_centers(self, predicts, x);
    array_dump(means);

    predicts = kmeans_predict(self, means, x);
	printf("predicts\n");
    array_dump(predicts);
	*/

	array* means;
    for(i = 0; i < iterations; i++) {
        mempool_free(mem, (char*)means);
        means = kmeans_calculate_centers(self, predicts, x);
        mempool_free(mem, (char*)predicts);
    	predicts = kmeans_predict(self, means, x);
    }

	means = kmeans_calculate_centers(self, predicts, x);

    return means;
}

int main(int argc, char** argv) {
    exception_begin();
    mylog_init(NULL, NULL, NULL);
    mylog_info(format("main start, %s %s", __DATE__, __TIME__));
	mempool_instance mem;
	mempool_init(&mem, 1024 * 1024 * 1024l * 10l);
	array_instance arr;
	array_init(&arr, &mem);

    // TSVからロード
    array* a = array_load_text(&arr, "./seeds_dataset.txt");

    // データ部（X）、解答ブ（Y） に分ける
    int x_columns[] = {0, 1, 2, 3, 4, 5, 6};
    int y_columns[] = {7};
    array_index* xindexes = array_index_from(&arr, x_columns, ARRAY_LENGTH(x_columns));
    array_index* yindexes = array_index_from(&arr, y_columns, ARRAY_LENGTH(y_columns));
    //array_index_dump(xindexes);
    //array_index_dump(yindexes);

    array* x = array_select_cols(&arr, a, xindexes);
    array* y = array_select_cols(&arr, a, yindexes);
    //array_dump(x);
    //array_dump(y);
	//array* xi = array_row_reference(&arr, x, 1);
	//array_dump(xi);

	array* means;
	array* predicts;
    kmeans_instance km;
    kmeans_init(&km, &arr, 3, x->cols);
    means = kmeans_train(&km, x);
	predicts = kmeans_predict(&km, means, x);

	puts("means dump");
	array_dump(means);
	puts("predicts dump");
	array_dump(predicts);
	puts("answer dump");
	array_dump(y);

    //array* x_columns = array_from();
    /*
	int y_columns[] = {7};
	array* x = array_select_cols(&arr, a, x_columns, ARRAY_LENGTH(x_columns));
	array* y = array_select_cols(&arr, a, y_columns, ARRAY_LENGTH(y_columns));
	//array_dump(a);
    //array_dump(x);
    //array_dump(y);
    mylog_info("calc start");

    kmeans_instance km;
    kmeans_init(&km, &arr, 3, ARRAY_LENGTH(x_columns));
    kmeans_train(&km, x);
    */

    mylog_info("calc end");
	mempool_end(&mem);

    return 0;
}

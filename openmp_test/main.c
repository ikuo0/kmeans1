
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

int main(int argc, char** argv) {
    exception_begin();
    mylog_init(NULL, NULL, NULL);
    mylog_info(format("main start, %s %s", __DATE__, __TIME__));
	mempool_instance mem;
	mempool_init(&mem, 1024 * 1024 * 1024 * 20l);
	array_instance instance;
	array_init(&instance, &mem);

    array* a = loaddata(&instance);
    //array_dump(a);
    mylog_info("calc start");
    //array* v = array_sum(&instance, a, array_axis_0);
    //array* v = array_mean(&instance, a, array_axis_0);
    //array* v = array_mean(&instance, a, array_axis_0);
    array* v = array_sqrt(&instance, a, array_axis_0);
    mylog_info("calc end");
    //array_dump(means);
    return 0;
}

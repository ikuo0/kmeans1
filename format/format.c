

#include <stdio.h>
#include <pthread.h>
#include <stdarg.h>

char format_buffer[4096];
//char *format_buffer = NULL;
pthread_mutex_t format_mutex;

void format_alloc(size_t size) {
	/*
	if(format_buffer != NULL) {
		free(format_buffer);
	}
	format_buffer = malloc(size * 2);
	*/
}

void format_free() {
	/*
	free(format_buffer);
	*/
}

void format_init(size_t default_size) {
	format_alloc(default_size);
	pthread_mutex_init(&format_mutex, NULL);
}

void format_end(size_t default_size) {
	format_free();
}

const char* format(const char* fmt, ...) {
	pthread_mutex_lock(&format_mutex);
	va_list args;
	va_start(args, fmt);
	vsprintf(format_buffer, fmt, args);
	va_end(args);
	pthread_mutex_unlock(&format_mutex);
	return format_buffer;
}

#ifdef FORMAT_UNITTEST

int main(int argc, char** argv) {
	format_init(1);
	fputs(format("aaaaaa  %d\n", 1), stdout);
	fputs(format("%s  segfsdfga%dwdf\n", "HOGE", 898989), stdout);
	return 0;
}

#endif

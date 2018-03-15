

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

FILE* mylog_stdout;
FILE* mylog_stdwarn;
FILE* mylog_stderr;
pthread_mutex_t mylog_mutex;

void mylog_init(FILE* standardout, FILE* warningout, FILE* errorout) {
	if(standardout == NULL) {
		mylog_stdout = stdout;
	}
	if(warningout == NULL) {
		mylog_stdwarn = stderr;
	}
	if(errorout == NULL) {
		mylog_stderr = stderr;
	}
	pthread_mutex_init(&mylog_mutex, NULL);
}

const char *mylog_timestr() {
	static char buffer[64];
	static struct timeval tv;
	time_t ut = time(NULL);
	struct tm * ts;
	ts = localtime(&ut);
	gettimeofday(&tv, NULL);
	sprintf(buffer,
		"%d/%d/%d %d:%d:%d.%03ld",
		ts->tm_year + 1900,
		ts->tm_mon + 1,
		ts->tm_mday,
		ts->tm_hour,
		ts->tm_min,
		ts->tm_sec,
		tv.tv_usec
	);
	return buffer;
}

void mylog_output(FILE* outstream, const char* level, const char *msg) {
	pthread_mutex_lock(&mylog_mutex);
	fputs(mylog_timestr(), outstream);
	fputs("\t", outstream);
	fputs(level, outstream);
	fputs("\t", outstream);
	fputs(msg, outstream);
	fputs("\n", outstream);
	pthread_mutex_unlock(&mylog_mutex);
}

void mylog_info(const char *msg) {
	mylog_output(mylog_stdout, "INFO", msg);
}

void mylog_warn(const char *msg) {
	mylog_output(mylog_stdwarn, "WARN", msg);
}

void mylog_err(const char *msg) {
	mylog_output(mylog_stderr, "ERROR", msg);
}


#ifdef MYLOG_UNITTEST

int main(int argc, char** argv) {
	mylog_init(NULL, NULL, NULL);
	mylog_info("info ほげほげ");
	mylog_warn("info ぴよぴよ");
	mylog_err("info エラー");
	return 0;
}

#endif

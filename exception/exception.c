
#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>

static jmp_buf exception_buffer;
static const char* exception_message;
static int exception_code;

void exception_begin() {
	if (setjmp(exception_buffer) == 0) {
		return;
	} else {
		printf("exception\tcode=%d\tmessage=%s\n", exception_code, exception_message);
		exit(exception_code);
	}
}

void exception_jump(const char* msg, int code) {
	exception_message = msg;
	exception_code = code;
	longjmp(exception_buffer, exception_code);
}


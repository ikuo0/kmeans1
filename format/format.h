
#ifndef _FORMAT_H_
#define _FORMAT_H_

extern const char* format(const char* fmt, ...)
	__attribute__((format(printf, 1, 2)));

#endif

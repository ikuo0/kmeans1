
#ifndef _MYLOG_H_
#define _MYLOG_H_

#include <stdio.h>

extern void mylog_init(FILE* standardout, FILE* warningout, FILE* errorout);
extern void mylog_info(const char *msg);
extern void mylog_warn(const char *msg);
extern void mylog_err(const char *msg);

#endif

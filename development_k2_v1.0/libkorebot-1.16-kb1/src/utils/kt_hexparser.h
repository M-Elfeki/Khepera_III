#ifndef _HEXPARSERINC
#define _HEXPARSERINC

#include<stdio.h>
#include<sys/types.h>

#define MIN(a,b) ((a)<(b)?(a):(b))
#define hexp_linelength 64

extern int hexp_start(FILE * progfile);
extern int hexp_next_line(FILE * progfile, int* code, int *datacount, int* address, char * data);
extern int hexp_parse_hex(FILE* progfile);
extern int hexp_valid_hex(FILE* progfile);
#endif /* _HEXPARSERINC */

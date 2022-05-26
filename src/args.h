#ifndef ARGS_H_
#define ARGS_H_

#include <stdio.h>

extern const char* const program_version;

typedef struct {
    char * region, *bam, *out;
    int size;
} arguments_t;

arguments_t parse_options(int argc, char **argv);


#endif // ARGS_H_

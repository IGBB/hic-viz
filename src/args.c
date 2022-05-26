#include "args.h"

#include "klib/ketopt.h"
#include <string.h>
#include <stdlib.h>



static ko_longopt_t longopts[] = {

    { "region-file", ko_required_argument, 'r' },
    { "plot-size", ko_required_argument, 's' },
    { "out", ko_required_argument, 'o' },

    {NULL, 0, 0}
  };


arguments_t parse_options(int argc, char **argv) {
  arguments_t arguments = {
                                .region = NULL,
                                .size  = 3000,
                                .bam = NULL,
                                .out = "/dev/stdout"
  };


  ketopt_t opt = KETOPT_INIT;

  int  c;
  FILE* tmp;
  while ((c = ketopt(&opt, argc, argv, 1, "r:s:", longopts)) >= 0) {
    switch(c){
      case 'o': arguments.out     = opt.arg;       break;
      case 'r': arguments.region  = opt.arg;       break;
      case 's': arguments.size    = atoi(opt.arg); break;
    };
  }

  if( opt.ind+1 == argc ){
      arguments.bam = argv[opt.ind];
  } else {
           perror("Expected a single positional argument\n");
          exit(EXIT_FAILURE);
  }

  return arguments;
}

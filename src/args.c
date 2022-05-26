#include "args.h"
#include "palette.h"
#include "klib/ketopt.h"
#include <string.h>
#include <stdlib.h>



static ko_longopt_t longopts[] = {

    { "region-file", ko_required_argument, 'r' },
    { "plot-size", ko_required_argument, 's' },
    { "out", ko_required_argument, 'o' },
    { "palette", ko_required_argument, 'p' },

    {NULL, 0, 0}
  };


arguments_t parse_options(int argc, char **argv) {
  arguments_t arguments = {
                                .region = NULL,
                                .size  = 3000,
                                .bam = NULL,
                                .out = "/dev/stdout",
                                .pal = rocket
  };


  ketopt_t opt = KETOPT_INIT;

  int  c;
  while ((c = ketopt(&opt, argc, argv, 1, "r:s:o:p:", longopts)) >= 0) {
    switch(c){
      case 'o': arguments.out     = opt.arg;       break;
      case 'r': arguments.region  = opt.arg;       break;
      case 's': arguments.size    = atoi(opt.arg); break;
      case 'p':
        if(strcmp(opt.arg, "magma") == 0)
          arguments.pal=magma;
        else if(strcmp(opt.arg, "inferno") == 0)
          arguments.pal=inferno;
        else if(strcmp(opt.arg, "mako") == 0)
          arguments.pal=mako;
        else if(strcmp(opt.arg, "rocket") == 0)
          arguments.pal=rocket;
         else {
           perror("--palette must be magma, inferno, mako, or rocket\n");
           exit(EXIT_FAILURE);
         }
        break;
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

#include "args.h"
#include "palette.h"
#include "klib/ketopt.h"
#include <string.h>
#include <stdlib.h>


const char* const help_message =
  "Usage: hic-viz [OPTION...] <BAM>\n"
  "hic-viz -- Simple hi-c contact plotting\n\n"
  "  -o, --out FILE         Filename to output png (default: stdout)\n"
  "  -r, --region FILE      List of sequences to include (default: all)\n"
  "                         Can be space, tab, or newline delimited\n"
  "  -s, --plot-size INT    Size of contact map in pixels (default: 3000)\n"
  "                         Actual plot will be larger due to sequence names\n"
  "  -p, --palette PALETTE  Palette of contact map (default: rocket)\n"
  "                         Valid options are magma, inferno, mako, or rocket\n"
  "                                     Valid options are phred64, 64, phred33, and 33.\n"
  "  -h, --help                      Give this help list\n"
  "Report bugs to github.com/IGBB/hic-viz.\n";


static ko_longopt_t longopts[] = {

    { "region", ko_required_argument, 'r' },
    { "plot-size", ko_required_argument, 's' },
    { "out", ko_required_argument, 'o' },
    { "palette", ko_required_argument, 'p' },
    { "help", ko_no_argument, 'h' },

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
  while ((c = ketopt(&opt, argc, argv, 1, "r:s:o:p:h", longopts)) >= 0) {
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
          fprintf(stderr,"--palette must be magma, inferno, mako, or rocket\n");
          fprintf(stderr, help_message);
          exit(EXIT_FAILURE);
        }
        break;
      case 'h':
        printf(help_message);
        exit(EXIT_SUCCESS);

    };
  }

  if( opt.ind+1 == argc ){
      arguments.bam = argv[opt.ind];
  } else {
          fprintf(stderr, "Expected a single positional argument\n");
          fprintf(stderr, help_message);
          exit(EXIT_FAILURE);
  }

  return arguments;
}

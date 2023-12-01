#include "args.h"
#include "palette.h"
#include "klib/ketopt.h"
#include <string.h>
#include <stdlib.h>


const char* const help_message =
  "Usage: hic-viz [OPTION...] <BAM>\n"
  "hic-viz -- Simple hi-c contact plotting\n\n"
  "  -o, --out FILE         Output file (default: stdout)\n"
  "  -t, --type TYPE        Image format [png,jpeg,tiff,bmp] (default: png)\n"
  "  -r, --region FILE      List of sequences to include (default: all)\n"
  "                             Can be space, tab, or newline delimited\n"
  "  -b, --bins INT         Number of bins for contact map (default: 3000)\n"
  "  -s, --scale INT        Scale multiplier for plot (default: 1)\n"
  "                             Must be greater than or equal to 1\n"
  "                             Actual plot will be larger than (bins * scale)\n"
  "                                 due to sequence names\n"
  "  -m, --max INT          Count to begin darkest color at (default determined by data)\n"
  "  -p, --palette PALETTE  Palette of contact map (default: rocket)\n"
  "                             Valid options are magma, inferno, mako, rocket, or grey\n"
  "  -h, --help             Give this help list\n"
  "Report bugs to github.com/IGBB/hic-viz.\n";


static ko_longopt_t longopts[] = {

    { "region", ko_required_argument, 'r' },
    { "bins", ko_required_argument, 'b' },
    { "scale", ko_required_argument, 's' },
    { "max", ko_required_argument, 'm' },
    { "out", ko_required_argument, 'o' },
    { "type", ko_required_argument, 't' },
    { "font", ko_required_argument, 'f' },
    { "palette", ko_required_argument, 'p' },
    { "help", ko_no_argument, 'h' },

    {NULL, 0, 0}
  };


arguments_t parse_options(int argc, char **argv) {
  arguments_t arguments = {
                                .region   = NULL,
                                .bins     = 3000,
                                .scale = 1,
                                .bam      = NULL,
                                .max      = 0,
                                .out      = "/dev/stdout",
                                .font     = "/usr/share/fonts/dejavu/DejaVuSans.ttf",
                                .pal      = rocket
  };


  ketopt_t opt = KETOPT_INIT;

  int  c;
  while ((c = ketopt(&opt, argc, argv, 1, "r:b:s:m:f:o:p:t:h", longopts)) >= 0) {
    switch(c){
      case 'o': arguments.out    = opt.arg;       break;
      case 'r': arguments.region = opt.arg;       break;
      case 'f': arguments.font   = opt.arg;       break;
      case 'b': arguments.bins   = atoi(opt.arg); break;
      case 's': arguments.scale  = atoi(opt.arg); break;
      case 'm': arguments.max    = atoi(opt.arg); break;
      case 't':
        if(strcmp(opt.arg, "png") == 0)
          arguments.type=png;
        else if(strcmp(opt.arg, "jpeg") == 0)
          arguments.type=jpeg;
        else if(strcmp(opt.arg, "tiff") == 0)
          arguments.type=tiff;
        else if(strcmp(opt.arg, "bmp") == 0)
          arguments.type=bmp;
        else {
          fprintf(stderr,"--type must be png, jpeg, tiff, or bmp\n");
          fprintf(stderr, help_message);
          exit(EXIT_FAILURE);
        }
        break;
      case 'p':
        if(strcmp(opt.arg, "magma") == 0)
          arguments.pal=magma;
        else if(strcmp(opt.arg, "inferno") == 0)
          arguments.pal=inferno;
        else if(strcmp(opt.arg, "mako") == 0)
          arguments.pal=mako;
         else if(strcmp(opt.arg, "rocket") == 0)
          arguments.pal=rocket;
       else if(strcmp(opt.arg, "grey") == 0)
          arguments.pal=grey;
        else {
          fprintf(stderr,"--palette must be magma, inferno, mako, rocket, or grey\n");
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

  if(arguments.scale < 1){
    fprintf(stderr, "Scale must be greater than or equal to one\n");
    fprintf(stderr, help_message);
    exit(EXIT_FAILURE);      
  }

  if(arguments.scale > 100){
    fprintf(stderr, "Size is no longer a command-line argument. "
            "Modified the given arguments to %d bins (-b) with a scale"
            "(-s) of 1.\n", arguments.scale);
    fprintf(stderr, help_message);
    
    arguments.bins = arguments.scale;
    arguments.scale = 1;
  }
  
  return arguments;
}

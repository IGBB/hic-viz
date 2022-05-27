#include <stdlib.h>
#include <stdio.h>

#include "htslib/htslib/sam.h"
#include "gd.h"
#include "gdfontmb.h"

#include "args.h"
#include "palette.h"

int get_bin(int offset[], int bin_size, int tid, int pos){
    long off;
    int bin;

    off = offset[tid] + pos;
    bin = off/bin_size;

    return bin;
}

int main(int argc, char *argv[]) {

    arguments_t args = parse_options(argc, argv);

    int i;

    htsFile* file;
    bam_hdr_t *header;


    file = hts_open(args.bam, "r");
    if(!file){
        fprintf(stderr, "Failed to open bam file '%s': %s\n",
                  args.bam, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(file->format.format != bam){
        fprintf(stderr, "File supplied is not in bam format. Current Format: %s",
                  hts_format_description(hts_get_format(file)));
        exit(EXIT_FAILURE);
    }
    header = sam_hdr_read(file);

    long total = 0;
    int *keep = calloc(header->n_targets, sizeof(int));

    if( args.region ){
        FILE* fh = fopen(args.region, "r");
        size_t len= 0;
        char buffer[2048];

        /* Try to make strtok safe'ish*/
        buffer[2047] = '\0';

        /* Read file, appending to the current buffer */
        while((len = fread(&(buffer[len]), 1, 2047-len, fh)) > 0) {
            /* loop through each name */
            char *tok = strtok(buffer, "\n \t");
            while(tok != NULL){
                /* look ahead, if null then copy tok to begining of buffer, else
                 * get tid from name and mark keep */
                char * new = strtok(NULL, "\n \t");
                if(new != NULL){
                    int tid = bam_name2id(header, tok);
                    keep[tid]=1;
                }else{
                    strcpy(buffer, tok);
                    len = strlen(buffer);
                }
                tok = new;
            }

        }
    }else{
        for(i = 0; i < header->n_targets; i++)
            keep[i]=1;
    }


    gdImagePtr tmp = gdImageCreate(300, 100);
    int tmpcolor = gdImageColorAllocate(tmp, 128,128,128);

    int plot_pad = 0;
    int *offset = calloc(header->n_targets, sizeof(int));
    for(i = 0; i < header->n_targets; i++){
        if(keep[i]){
            offset[i] = total;
            total += header->target_len[i];

            int len = strlen(sam_hdr_tid2name(header, i));
            if(len > plot_pad) plot_pad = len;

        }
    }
    plot_pad = plot_pad * 7 + 10 ;

    int bin_size = (total/args.size) + 1;

    int * counts = calloc(args.size*args.size, sizeof(int));

    bam1_t * read = bam_init1();
    while(sam_read1(file, header, read) >= 0) {
        if(keep[read->core.tid] && keep[read->core.mtid]){

            long x, y;
            x = offset[read->core.tid] + read->core.pos;
            y = offset[read->core.mtid] + read->core.mpos;

            int bin_x, bin_y;
            bin_x = x/bin_size;
            bin_y = y/bin_size;
            counts[bin_x*args.size + bin_y] ++;
        }
    }

    /* for( i = 0; i < args.size*args.size; i++ ) */
    /*     printf("%d%c", counts[i], " \n"[((i+1)%args.size) == 0]); */

    int max = 0, num=0;
    long tot = 0;
    for( i = 0; i < args.size*args.size; i++ ){
        if(counts[i] > 1){
            num++;
            tot += counts[i];
        }
        if(max < counts[i]) max = counts[i];
    }

    printf("Max = %d\n", max);
    max = tot/num;
    printf("Mean = %d\n", max);

    gdImagePtr img = gdImageCreate(args.size+plot_pad, args.size+plot_pad);
    int * colors = load_palette(img, args.pal);

    for( i = 0; i < args.size*args.size; i++ ){
        int color = 255*counts[i]/max;
        if(color > 255) color=255;
        color=colors[color];

        gdImageSetPixel(img, i/args.size, i%args.size, color);


//        printf("%d%c", 255*counts[i]/max, " \n"[((i+1)%args.size) == 0]);
    }

    int line_color = gdImageColorClosest(img, 128,128,128);
    int last = 0;
    gdFontPtr font = gdFontGetMediumBold();
    for( i = 0; i < header->n_targets; i++ ){
        if(keep[i]){
            int bin = offset[i]/bin_size;

            if(bin - last <= 0) continue;

            gdImageLine( img, bin, 0, bin, args.size, line_color );
            gdImageLine( img, 0, bin, args.size, bin, line_color );

            unsigned char * name = sam_hdr_tid2name(header, i);

            int brect[8];
            gdImageStringFT(img, brect, line_color,
                            "/usr/share/fonts/dejavu/DejaVuSans.ttf",
                            24, -0.785, args.size+5, (bin+last)/2,
                          name);
            gdImageStringFT(img, brect, line_color,
                            "/usr/share/fonts/dejavu/DejaVuSans.ttf",
                            24, -0.785, (bin+last)/2, args.size+5,
                          name);

            last = bin;
        }
    }

    FILE* pngout = fopen(args.out, "wb");
    gdImagePng(img, pngout);
    fclose(pngout);

    /* printf("Total length: %ld\n", total); */
    /* printf("Size of bins: %ld\n", bin_size); */
    /* printf("   Max count: %d\n" , max); */
    return 0;
}

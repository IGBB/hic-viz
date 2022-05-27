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

        /* Read file, appending to the current buffer */
        while(fscanf(fh, "%2047s", buffer) == 1 ) {
            int tid = bam_name2id(header, buffer);
            keep[tid]=1;
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

            int brect[8];
            gdImageStringFT(tmp, brect, tmpcolor,
                            "/usr/share/fonts/dejavu/DejaVuSans.ttf",
                            24, 0, 0, 0,
                          sam_hdr_tid2name(header, i));

            /* test x */
            int min, max, j;
            min = max = brect[0];

            for(j = 2; j < 8; j+=2){
                if(min > brect[j])
                    min = brect[j];
                if(max < brect[j])
                    max = brect[j];
            }

            int len = max - min;
            if(plot_pad < len)
                plot_pad = len;

            /* test y */
            min = max = brect[1];

            for(j = 3; j < 8; j+=2){
                if(min > brect[j])
                    min = brect[j];
                if(max < brect[j])
                    max = brect[j];
            }

            len = max - min;
            if(plot_pad < len)
                plot_pad = len;


        }
    }
    gdImageDestroy(tmp);

    /* pad with 10px */
    plot_pad += 10 ;

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
    gdFontPtr font = gdFontGetMediumBold();
    for( i = 0; i < header->n_targets; i++ ){
        if(keep[i]){
            int bin = offset[i]/bin_size;
            int next = (offset[i] + header->target_len[i])/bin_size;

            if(next - bin <= 0) continue;

            gdImageLine( img, bin, 0, bin, args.size, line_color );
            gdImageLine( img, 0, bin, args.size, bin, line_color );

            unsigned char * name = sam_hdr_tid2name(header, i);

            int brect[8];
            gdImageStringFT(img, brect, line_color,
                            "/usr/share/fonts/dejavu/DejaVuSans.ttf",
                            24,0 , args.size+5, (bin+next)/2,
                          name);
            gdImageStringFT(img, brect, line_color,
                            "/usr/share/fonts/dejavu/DejaVuSans.ttf",
                            24, -1.57, (bin+next)/2, args.size+5,
                          name);

        }
    }

    FILE* out = fopen(args.out, "wb");

    switch(args.type){
        case png:  gdImagePng(img, out);     break;
        case jpeg: gdImageJpeg(img, out, 95); break;
        case tiff: gdImageTiff(img, out);    break;
        case bmp:  gdImageBmp(img, out, 0);  break;
    }
    fclose(out);

    gdImageDestroy(img);
    /* printf("Total length: %ld\n", total); */
    /* printf("Size of bins: %ld\n", bin_size); */
    /* printf("   Max count: %d\n" , max); */
    return 0;
}

#include <stdlib.h>
#include <stdio.h>

#include "htslib/htslib/sam.h"
#include "gd.h"

#include "args.h"
#include "palette.h"

int get_bin(int offset[], int bin_size, int tid, int pos){
    long off;
    int bin;

    off = offset[tid] + pos;
    bin = off/bin_size;

    return bin;
}
htsFile* htsOpen(char* filename){
    htsFile* file = hts_open(filename, "r");

    /* Did file open */
    if(!file){
        fprintf(stderr, "Failed to open bam file '%s': %s\n",
                  filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Is file a bam */
    if(file->format.format != bam){
        fprintf(stderr, "File supplied is not in bam format. Current Format: %s",
                  hts_format_description(hts_get_format(file)));
        exit(EXIT_FAILURE);
    }

    return file;
}

void read_filter_file(bam_hdr_t * header, int keep[], char* file){
    FILE* fh = fopen(file, "r");
    char buffer[2048];

    /* Read file, appending to the current buffer */
    while(fscanf(fh, "%2047s", buffer) == 1 ) {
        int tid = bam_name2id(header, buffer);
        keep[tid]=1;
    }
}

int get_padding(bam_hdr_t *header, int keep [], char* font){
    int padding = 0 ;
    int i, j, min, max, len, brect[8];
    gdImagePtr tmp = gdImageCreate(300, 100);
    int tmpcolor = gdImageColorAllocate(tmp, 128,128,128);

    for(i = 0; i < header->n_targets; i++){
        if(keep[i]){

            gdImageStringFT(tmp, brect, tmpcolor,
                            font,
                            24, 0, 0, 0,
                          sam_hdr_tid2name(header, i));

            /* test x */
            min = max = brect[0];

            for(j = 2; j < 8; j+=2){
                if(min > brect[j])
                    min = brect[j];
                if(max < brect[j])
                    max = brect[j];
            }

            len = max - min;
            if(padding < len)
                padding = len;

            /* test y */
            min = max = brect[1];

            for(j = 3; j < 8; j+=2){
                if(min > brect[j])
                    min = brect[j];
                if(max < brect[j])
                    max = brect[j];
            }

            len = max - min;
            if(padding < len)
                padding = len;


        }
    }
    gdImageDestroy(tmp);


    return padding + 10;
}

long * get_offsets (bam_hdr_t *header, int keep []){
    long * offsets = malloc((header->n_targets + 1 ) * sizeof(long));
    long total = 0;
    int i;

    for(i = 0; i < header->n_targets; i++){
        offsets[i] = total;
        if(keep[i]) total+=header->target_len[i];
    }

    offsets[i] = total;
    return offsets;
}

int main(int argc, char *argv[]) {

    arguments_t args = parse_options(argc, argv);

    int i;

    htsFile* file = htsOpen(args.bam);
    bam_hdr_t *header = sam_hdr_read(file);

    int *keep = calloc(header->n_targets, sizeof(int));

    if( args.region ){
        read_filter_file(header, keep, args.region);
    }else{
        for(i = 0; i < header->n_targets; i++)
            keep[i]=1;
    }

    int padding =  get_padding(header, keep, args.font);

    long * offset = get_offsets(header, keep);
    long total = offset[header->n_targets];
    int bin_size = (total/args.size) + 1;

    int * counts = calloc(args.size*args.size, sizeof(int));

    bam1_t * read = bam_init1();
    while(sam_read1(file, header, read) >= 0) {

        long x, y;
        x = offset[read->core.tid] + read->core.pos;
        y = offset[read->core.mtid] + read->core.mpos;

        int bin_x, bin_y;
        bin_x = x/bin_size;
        bin_y = y/bin_size;

        if(keep[read->core.tid] && keep[read->core.mtid])
            counts[bin_x*args.size + bin_y] ++;

    }

    /* for( i = 0; i < args.size*args.size; i++ ) */
    /*     printf("%d%c", counts[i], " \n"[((i+1)%args.size) == 0]); */

    /* Get number and total counts of entries greater than 1 */
    int num=0;
    long tot = 0;
    for( i = 0; i < args.size*args.size; i++ ){
        if(counts[i] > 1){
            num++;
            tot += counts[i];
        }
    }

    /* Adjust max color to twice the average of entries > 1 */
    int max = 2*tot/num;

    /* Create image and palette */
    gdImagePtr img = gdImageCreate(args.size+padding, args.size+padding);
    int * colors = load_palette(img, args.pal);

    /* Draw contact map */
    for( i = 0; i < args.size*args.size; i++ ){
        int color = 255*counts[i]/max;
        if(color > 255) color=255;
        color=colors[color];

        gdImageSetPixel(img, i/args.size, i%args.size, color);
    }


    /* Draw lines and labels, skipping if too close (10px) */
    int line_color = gdImageColorClosest(img, 128,128,128);
    for( i = 0; i < header->n_targets; i++ ){
        if(keep[i]){
            int bin = offset[i]/bin_size;
            int next = (offset[i] + header->target_len[i])/bin_size;

            if(next - bin <= 10) continue;

            gdImageLine( img, bin, 0, bin, args.size, line_color );
            gdImageLine( img, 0, bin, args.size, bin, line_color );

            const char * name = sam_hdr_tid2name(header, i);

            int brect[8];
            gdImageStringFT(img, brect, line_color, args.font,
                            24,0 , args.size+5, (bin+next)/2,
                          name);
            gdImageStringFT(img, brect, line_color, args.font,
                            24, -1.57, (bin+next)/2, args.size+5,
                          name);

        }
    }

    /* Write image */
    FILE* out = fopen(args.out, "wb");
    switch(args.type){
        case png:  gdImagePng(img, out);     break;
        case jpeg: gdImageJpeg(img, out, 95); break;
        case tiff: gdImageTiff(img, out);    break;
        case bmp:  gdImageBmp(img, out, 0);  break;
    }
    fclose(out);

    gdImageDestroy(img);
    return 0;
}

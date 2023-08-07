#include <stdlib.h>
#include <stdio.h>

#include "htslib/htslib/sam.h"
#include "gd.h"

#include "args.h"
#include "palette.h"

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


/* region_t data struct
 ** @field str - display string
 ** @field tid - target id in the bam file
 ** @field pos - region position (.beg and .end)
 * */
typedef struct REGION {
    const char * str;
    int tid, start_bin, dir;
    long remaining;
    hts_pair_pos_t pos;
    struct REGION *order, *page;
} region_t ;

/* Parses file of samtools style regions to set order of visible sequences
** @param header - full bam header
**
** @param file --- file containing samtools tyle regions (e.g. Chr1:100-1000,
**                 Chr1:100-, Chr1), separated by whitespace, in disply order.
**                 If a file in not given, or cannot be opened, all sequences
**                 will be displayed in the order given in the header.
**
** @return ------- returns a pointer to reglist_t (kvec of regions), where .a is
**                 the array of regions and .n is the number of regions in
**                 array.
 */
region_t* get_reglist(bam_hdr_t * header, char* file){
    region_t *reglist, *cur, *prev;

    if(file){
        FILE* fh = fopen(file, "r");
        /* Read next target */
        reglist = malloc(sizeof(region_t));
        prev = cur = reglist;
        char * buffer = malloc(sizeof(char) * 2048);

        while(fscanf(fh, "%2047s", buffer) == 1 ) {
            cur->str = buffer;
            cur->dir=1;

            // Reverse direction if region starts with '-'
            if(buffer[0] == '-'){
                cur->dir=-1;
                buffer++;
            }

            if(sam_parse_region(header,
                                buffer,
                                &(cur->tid),
                                &(cur->pos.beg),
                                &(cur->pos.end), 0) == NULL){
                fprintf(stderr, "Can't parse region: '%s'\n", buffer);
                exit(EXIT_FAILURE);
            }

            if(cur->pos.end > sam_hdr_tid2len(header, cur->tid))
                cur->pos.end = sam_hdr_tid2len(header, cur->tid);

            cur->order = malloc(sizeof(region_t));
            prev = cur;
            cur = cur->order;
            buffer = malloc(sizeof(char) * 2048);
        }
        free(buffer);
        free(cur);
        prev->order = NULL;
    }else{

        reglist = malloc(sizeof(region_t) * header->n_targets);

        /* Set order to match bam header */
        int i;
        for(i = 0; i < header->n_targets; i++){
            reglist[i].str = sam_hdr_tid2name(header, i);
            reglist[i].tid = i;
            reglist[i].dir = 1;
            reglist[i].pos.beg = 0;
            reglist[i].pos.end = sam_hdr_tid2len(header, i);
            reglist[i].order = reglist + (i + 1);
        }
        reglist[header->n_targets-1].order = NULL;
    }

    return reglist;
}

/* Get the padding needed to fit the largest sequence name in the plot
 ** @param reglist - pointer to region list (reglist_t).
 ** @param font ---- FreeType font used
 ** @return -------- size needed to pad the image to fit all text
 */

int get_padding(region_t* reglist, char* font){
    region_t* cur  = reglist;
    int padding = 0 ;
    int i, j, min, max, len, brect[8];

    /* Create tmp image and color */
    gdImagePtr tmp = gdImageCreate(300, 100);
    int tmpcolor = gdImageColorAllocate(tmp, 128,128,128);

    /* loop through all sequences to get the max padding needed.
     **
     ** TODO: optmize by finding the longest string and only one bounding box
     ** */
    while(cur != NULL) {
        gdImageStringFT(tmp, brect, tmpcolor,
                        font, 24, 0, 0, 0, cur->str);

        /* test x then y */
        for(i = 0; i <= 1; i++){
            min = max = brect[i];

            for(j = (i+2); j < 8; j+=2){
                if(min > brect[j]) min = brect[j];
                if(max < brect[j]) max = brect[j];
            }

            len = max - min;
            if(padding < len) padding = len;
        }

        cur = cur->order;
    }
    gdImageDestroy(tmp);


    return padding + 10;
}

/* Create array of linked pages */
region_t ** create_pages(sam_hdr_t * head, region_t * reglist, int number_of_bins, long *bin_size) {
    region_t* cur = reglist;
    region_t ** pages = calloc(head->n_targets, sizeof(region_t*));
    int bin;
    long remaining;

    *bin_size = 0;

    /* Loop throu all ranges */
    while(cur != NULL) {
        /* get total number of bases need to visualize */
        (*bin_size) += (cur->pos.end - cur->pos.beg);
        cur = cur->order;
    }
    /* convert total number of bases to size of each bin in matrix */
    (*bin_size) /= number_of_bins;

     /* Loop throu all ranges again to assign starting bin */
    bin = 0;
    remaining = 0;
    cur = reglist;
    while(cur != NULL) {
        cur->start_bin = bin;
        cur->remaining = remaining;
        cur->page = pages[cur->tid];
        pages[cur->tid] = cur;

        remaining += (cur->pos.end - cur->pos.beg);

        bin += remaining/(*bin_size);
        remaining %= (*bin_size);

        cur = cur->order;
    }

    return pages;
};

int pos2bin(region_t * page, int pos, long bin_size){
    region_t * cur = page;
    while(cur != NULL){
        if(pos >= cur->pos.beg && pos < cur->pos.end){
            int bin = cur->start_bin;
            if(cur->dir == 1 ){
                 bin += ((cur->remaining + (pos - cur->pos.beg))/bin_size);
            }else{
                 bin += ((cur->remaining + (cur->pos.end - pos))/bin_size);
            }
            return bin;
        }
        cur = cur->page;
    }

    /* if no overlapping region is found, return -1 */
    return -1;
}

int main(int argc, char *argv[]) {

    arguments_t args = parse_options(argc, argv);

    int i;
    long bin_size;

    htsFile* file = htsOpen(args.bam);
    bam_hdr_t *header = sam_hdr_read(file);

    region_t* reglist = get_reglist(header, args.region);
    region_t** pages = create_pages(header, reglist, args.size, &bin_size);
    int * counts = calloc((args.size+1)*(args.size+1), sizeof(int));

    bam1_t * read = bam_init1();
    while(sam_read1(file, header, read) >= 0) {
        long x, y;
        x = pos2bin(pages[read->core.tid],  read->core.pos,  bin_size);
        y = pos2bin(pages[read->core.mtid], read->core.mpos, bin_size);

        if( x >= 0 && y >= 0) counts[x*args.size + y] ++;
    }

    /* for( i = 0; i < args.size*args.size; i++ ) */
    /*      printf("%d%c", counts[i], " \n"[((i+1)%args.size) == 0]); */

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

    int padding =  get_padding(reglist, args.font);
    /* Create image and palette */
    gdImagePtr img = gdImageCreate(args.size+padding, args.size+padding);
    int * colors = load_palette(img, args.pal);

    /* Draw contact map */
    for( i = 0; i < args.size*args.size; i++ ){
        int color = 255L*counts[i]/max;
        if(color > 255) color=255;
        color=colors[color];

        gdImageSetPixel(img, i/args.size, i%args.size, color);
    }


    /* Draw lines and labels, skipping if too close (10px) */
    int line_color = gdImageColorClosest(img, 128,128,128);
    region_t* cur = reglist;
    while(cur != NULL){
        int beg, end;
        if(cur->dir == 1){
            beg = pos2bin(pages[cur->tid], cur->pos.beg, bin_size);
            end = pos2bin(pages[cur->tid], cur->pos.end-1, bin_size);
        }else{
            beg = pos2bin(pages[cur->tid], cur->pos.end-1, bin_size);
            end = pos2bin(pages[cur->tid], cur->pos.beg, bin_size);
        }

        if(end - beg <= 10 ) goto NEXT;

        gdImageLine( img, beg, 0, beg, args.size, line_color );
        gdImageLine( img, 0, beg, args.size, beg, line_color );

        const char * name = cur->str;

        int brect[8];
        gdImageStringFT(img, brect, line_color, args.font,
                        24,0 , args.size+5, (beg+end)/2,
                        name);
        gdImageStringFT(img, brect, line_color, args.font,
                        24, -1.57, (beg+end)/2, args.size+5,
                        name);
    NEXT:
        cur = cur->order;
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

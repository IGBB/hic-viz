#include <stdlib.h>
#include <stdio.h>

#include "htslib/htslib/sam.h"
#include "/apps/gd-2.3.0/include/gd.h"

#include "args.h"

#define RGBr(code) (((code) >> 16) & 0xFF)
#define RGBg(code) (((code) >> 8) & 0xFF)
#define RGBb(code) ((code) & 0xFF)

const int rocket[255] = {
0x03051A, 0x04051A, 0x05061B, 0x06071C, 0x07071D, 0x08081E,
0x0A091F, 0x0B0920, 0x0D0A21, 0x0E0B22, 0x100B23, 0x110C24,
0x130D25, 0x140E26, 0x160E27, 0x170F28, 0x180F29, 0x1A102A,
0x1B112B, 0x1D112C, 0x1E122D, 0x20122E, 0x211330, 0x221331,
0x241432, 0x251433, 0x271534, 0x281535, 0x2A1636, 0x2B1637,
0x2D1738, 0x2E1739, 0x30173A, 0x31183B, 0x33183C, 0x34193D,
0x35193E, 0x37193F, 0x381A40, 0x3A1A41, 0x3C1A42, 0x3D1A42,
0x3F1B43, 0x401B44, 0x421B45, 0x431C46, 0x451C47, 0x461C48,
0x481C48, 0x491D49, 0x4B1D4A, 0x4C1D4B, 0x4E1D4B, 0x501D4C,
0x511E4D, 0x531E4D, 0x541E4E, 0x571E4F, 0x581E4F, 0x591E50,
0x5B1E51, 0x5C1E51, 0x5F1F52, 0x601F52, 0x611F53, 0x631F53,
0x641F54, 0x671F54, 0x681F55, 0x691F55, 0x6C1F56, 0x6D1F56,
0x6F1F57, 0x701F57, 0x711F57, 0x741F58, 0x751F58, 0x761F58,
0x791F59, 0x7A1F59, 0x7C1F59, 0x7E1F5A, 0x801E5A, 0x811E5A,
0x831E5A, 0x851E5A, 0x861E5B, 0x881E5B, 0x8A1E5B, 0x8B1D5B,
0x8D1D5B, 0x8F1D5B, 0x911D5B, 0x921C5B, 0x941C5B, 0x961C5B,
0x971C5B, 0x991B5B, 0x9B1B5B, 0x9D1B5B, 0x9E1A5B, 0xA01A5B,
0xA21A5B, 0xA3195B, 0xA5195B, 0xA7195A, 0xA9185A, 0xAA185A,
0xAC185A, 0xAE1759, 0xAF1759, 0xB11759, 0xB31758, 0xB41658,
0xB61657, 0xB81657, 0xB91657, 0xBB1656, 0xBD1656, 0xBE1654,
0xC01654, 0xC11754, 0xC31753, 0xC41753, 0xC61951, 0xC71951,
0xC91951, 0xCB1B4F, 0xCC1B4E, 0xCE1D4E, 0xCE1E4E, 0xD01F4C,
0xD2204C, 0xD2214C, 0xD4214A, 0xD62349, 0xD72549, 0xD82649,
0xD82847, 0xDA2846, 0xDC2A46, 0xDD2C46, 0xDE2D44, 0xDF2F44,
0xE03044, 0xE13242, 0xE23442, 0xE33541, 0xE43741, 0xE53940,
0xE63A40, 0xE73C3F, 0xE83E3F, 0xE8403E, 0xE9413E, 0xEA433E,
0xEB453E, 0xEB473E, 0xEC493E, 0xEC4B3E, 0xED4D3E, 0xED4F3E,
0xEE513F, 0xEE533F, 0xEF5540, 0xEF5740, 0xEF5941, 0xF05B42,
0xF05D42, 0xF05F43, 0xF16144, 0xF16345, 0xF16546, 0xF26747,
0xF26848, 0xF26A49, 0xF26C4A, 0xF26E4C, 0xF3704D, 0xF3734E,
0xF3744F, 0xF37551, 0xF37752, 0xF47954, 0xF47C55, 0xF47D57,
0xF47E58, 0xF4805A, 0xF4835B, 0xF4845D, 0xF4855E, 0xF58760,
0xF58A61, 0xF58B63, 0xF58C64, 0xF58F66, 0xF59067, 0xF59268,
0xF5946B, 0xF5966C, 0xF5976E, 0xF59970, 0xF69B71, 0xF69C73,
0xF69E75, 0xF6A077, 0xF6A178, 0xF6A37A, 0xF6A47C, 0xF6A67E,
0xF6A880, 0xF6A981, 0xF6AB83, 0xF6AD85, 0xF6AE87, 0xF6B089,
0xF6B18B, 0xF6B38D, 0xF6B48F, 0xF6B691, 0xF6B893, 0xF6B995,
0xF6BB97, 0xF6BC99, 0xF6BE9B, 0xF6BF9D, 0xF6C19F, 0xF7C2A2,
0xF7C4A4, 0xF7C6A6, 0xF7C7A8, 0xF7C9AA, 0xF7CAAC, 0xF7CCAF,
0xF7CDB1, 0xF7CFB3, 0xF7D0B5, 0xF8D1B8, 0xF8D3BA, 0xF8D4BC,
0xF8D6BE, 0xF8D7C0, 0xF8D9C3, 0xF8DAC5, 0xF8DCC7, 0xF9DDC9,
0xF9DFCB, 0xF9E0CD, 0xF9E2D0, 0xF9E3D2, 0xF9E5D4, 0xFAE6D6,
0xFAE8D8, 0xFAE9DA, 0xFAEBDD
};


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

    int *offset = calloc(header->n_targets, sizeof(int));
    for(i = 0; i < header->n_targets; i++){
        if(keep[i]){
            offset[i] = total;
            total += header->target_len[i];
        }
    }

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

    gdImagePtr img = gdImageCreate(args.size, args.size);





    int colors[255];
//    colors[0] = gdImageColorAllocate(img, 255,255,255);
    for(i = 0; i < 255; i++)
        colors[i] = gdImageColorAllocate(img,
                                         RGBr(rocket[254-i]),
                                         RGBg(rocket[254-i]),
                                         RGBb(rocket[254-i]));


    for( i = 0; i < args.size*args.size; i++ ){
        int color = 255*counts[i]/max;
        if(color >= 255) color=254;
        color=colors[color];

        gdImageSetPixel(img, i/args.size, i%args.size, color);


//        printf("%d%c", 255*counts[i]/max, " \n"[((i+1)%args.size) == 0]);
    }

    int line_color = gdImageColorAllocate(img, 128,128,128);
    for( i = 1; i < header->n_targets; i++ ){
        if(keep[i]){
            int last = offset[i-1]/bin_size;
            int bin = offset[i]/bin_size;

            if(bin - last < 0) continue;

            gdImageLine( img, bin, 0, bin, args.size, line_color );
            gdImageLine( img, 0, bin, args.size, bin, line_color );
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

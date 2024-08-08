#include <stdio.h>
#include <string.h>
#include <math.h>

#define SI3 61.735
#define DO0 65.406
#define LE0 73.416
#define MI0 82.407
#define FA0 87.307
#define SO0 97.999
#define SS0 103.826
#define LA0 110.000
#define SI0 123.471
#define DO1 130.813
#define LE1 146.832
#define MI1 164.814
#define FA1 174.614
#define SO1 195.998
#define SS1 207.652
#define LA1 220.000
#define SI1 246.942
#define DO2 261.63

int main(void){

    char *drm = "SIDOLEMIFASOSSLASIDOLEMIFASOSSLASIDO";
    double drmh[18] = {SI3,DO0, LE0, MI0, FA0, SO0,SS0, LA0, SI0,
                        DO1, LE1, MI1, FA1, SO1,SS1, LA1, SI1, DO2}; 

    for(int i=0; i<18; i++){
        //printf("%d\n",i);
        printf("`define %c%c%d %5.0lf\n", drm[i*2],drm[i*2+1], (i<7)?0:1, round((24000000.0/drmh[i])/2-1));
    }
    printf("`define RE0 0\n");

    return 0;
}
#include <stdio.h>
#include <string.h>
#include <math.h>

#define DO0 261.63
#define LE0 293.67
#define MI0 329.63
#define FA0 349.23
#define SO0 392.00
#define LA0 440.00
#define SI0 493.88
#define DO1 523.23
#define LE1 587.34
#define MI1 659.25
#define FA1 698.45
#define SO1 783.98
#define LA1 879.99
#define SI1 987.75

int main(void){

    char *drm = "DOLEMIFASOLASIDOLEMIFASOLASI";
    double drmh[14] = {DO0, LE0, MI0, FA0, SO0, LA0, SI0,
                        DO1, LE1, MI1, FA1, SO1, LA1, SI1}; 

    for(int i=0; i<14; i++){
        //printf("%d\n",i);
        printf("`define %c%c%d %5.0lf\n", drm[i*2],drm[i*2+1], (i<7)?0:1, round((24000000.0/drmh[i])/2-1));
    }
    printf("`define RE0 0\n");

    return 0;
}
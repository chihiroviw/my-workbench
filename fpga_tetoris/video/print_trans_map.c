#include <stdio.h>

int main(void){

    for(int i=0; i<48; i++){
        //printf("                map[bx*4 + (by+%d)*128 + %d] <= block[%d]==1?1:0;\n",
         //           i/16, i%16,i);
        printf("                map[xyoff(bx,by,%d,%d)] <= (map>>xyoff(bx,by,%d,%d))&3'b111|block[%d];\n",
                    i/12, i%12,i/12,i%12,i);

    }
    return 0;
}
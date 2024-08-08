#include <stdio.h>

int main(void){
    for(int i=2; i<=18; i++){
        printf("map[%d:%d] <= xline=> n ? map[%d:%d]:map[%d:%d];\n"
        ,i*8-1, i*8-8,(i-1)*8,(i-1)*8-8,i*8-1, i*8-8);
    }
    return 0;
}
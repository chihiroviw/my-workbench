#include <stdio.h>

int main(void){
    for(int i=0; i<15; i++){
        if(i==0 || i==14){
            printf("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
        }else{
            printf("F00000000000F0000000000000000000");
        }
    }
    return 0;
}
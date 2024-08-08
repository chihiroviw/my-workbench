#include <stdio.h>

int main(void){
    //0 natural //2'b00
    //1 right   //2'b01
    //2 dark    //2'b10
    for(int i=0; i<32; i++){
        printf("64'h");
        for(int j=0; j<32; j++){
            if(i==j||i==-j+31){
                printf("00");//0
            }else if((i<j)&&(i<=4||28<=j)){
                printf("10");//2
            }else if(28<=i||j<=4){
                printf("01");//1
            }else{
                printf("00");//0
            }
        }
        printf(",\\\n");
    }

    return 0;
}
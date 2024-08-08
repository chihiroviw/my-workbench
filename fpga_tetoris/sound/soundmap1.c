#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SD1 "MI08MI18MI08MI18MI08MI18MI08MI18"
#define SD2 "LA08LA18LA08LA18LA08LA18LA08LA18"
#define SD3 "SS08SS18SO08SO18MI08MI18MI08MI18"
#define SD4 "LA08LA18LA08LA18LA08LA18SI08DO18"
#define SD5 "LE18LE08RE08LE08RE08LE08SO08FA08"
#define SD6 "DO08DO18RE08DO18DO08DO18LE08DO08"
#define SD7 "SI38SI08SI38SI08MI08MI18MI08MI18"
#define SD8 "LA08LA18LA08LA18LA08LA18LA04"

int main(void){
    char *sd = SD1 SD2 SD3 SD4 SD5 SD6 SD7 SD8;
    
    int i=0;
    for(; sd[0]!='\0'; sd+=4){
        char num[2];
        num[0] = sd[3];
        num[1] = '\0';
        int n = atoi(num);

        if(n == 5){
            for(int j=0; j<6; j++){
                printf("%d:melody = `%c%c%c ;\n",i,sd[0],sd[1],sd[2]);
                i++;
            }
        }else if(n == 6){
            printf("%d:melody = `%c%c%c ;\n",i,sd[0],sd[1],sd[2]);
            i++;
        }else{
            for(int j=0; j<16/n; j++){
                printf("%d:melody = `%c%c%c ;\n",i,sd[0],sd[1],sd[2]);
                i++;
            }
        }
    }

    return 0;
}
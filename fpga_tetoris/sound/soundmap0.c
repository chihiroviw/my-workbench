#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define SD1 "MI14SI08DO18LE18MI16LE16DO18SI08"
#define SD2 "LA04LA08DO18MI14LE18DO18"
#define SD3 "SI04SI08DO18LE14MI14"
#define SD4 "DO14LA04LA04RE04"
#define SD5 "RE08LE14FA18LA18LA16LA16SO18FA18"
#define SD6 "MI15DO18MI18LA06SO06LE18DO18"
#define SD7 "SI04SI08DO18LE14MI14"
#define SD8 "DO14LA04LA04RE04"

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
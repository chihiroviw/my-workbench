#include <stdio.h>
#include <stdlib.h>

#define SIZE    (3072)

float in[SIZE];
float out[SIZE];

int main(int argc, char **argv)
{
  int i,loop;
  int ret;

  for(i=0;i<SIZE;i+=2){
    in[i]=i+1;
    in[i+1]=-(i+2);
  }

  /* Calculate absolute values */
  for(loop=0;loop<1000000;loop++){
    for (i = 0; i < SIZE; i++) {
      if (in[i] > 0) {
	out[i] = in[i];
      } else {
	out[i] = in[i] * -1;
      }
    }
  }

  for (i = SIZE-3; i < SIZE; i++) {
    printf("in [%02d]=%0.0f\n", i, in[i]);
    printf("out[%02d]=%0.0f\n", i, out[i]);
  }
  
  return 0;
}

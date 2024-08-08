#include <stdlib.h>
#include <stdio.h>
#include <values.h>

int main(int argc, char **argv){
  int i,r=1;
  float s=0.0,a;

  if(argc<2 || argc>3){
    printf("usage : %s flag number_of_iterations\n",argv[0]);
    printf("   flag 0 : maximum value\n");
    printf("        1 : NaN\n");
    printf("        2 : Overflow(inf)\n");
    printf("        3 : Zero\n");
    printf("        4 : add 1\n");
    printf("        5 : add 0.1\n");
    exit(1);
  }

  if(argc>=3){
    sscanf(argv[2],"%d",&r);
    printf("number of repeat is %d\n",r);
  }

  switch(argv[1][0]){
  case '0':
    s = MAXFLOAT;
    printf("s = %16.8e (%08x)\n",s,*((unsigned int *)&s));
    s = MINFLOAT;
    printf("s = %16.8e (%08x)\n",s,*((unsigned int *)&s));
    break;
  case '1':
    s = 0.0;
    printf("s = %16.8e (%08x)\n",s,*((unsigned int *)&s));
    break;
  case '2':
    s = 0.0;
    printf("s = %16.8e (%08x)\n",s,*((unsigned int *)&s));
    break;
  case '3':
    s = 0.0;
    printf("s = %16.8e (%08x)\n",s,*((unsigned int *)&s));
    break;
  case '4':
    a=1;
    for(i=0; i<r; i++){
      s += a;
    }
    printf("s = %16.8e\n",s);
    break;
  case '5':
    a=0.1;
    for(i=0; i<r; i++){
      s += a;
    }
    printf("s = %16.8e\n",s);
    break;
  default:
    break;
  }

  return 0;
}

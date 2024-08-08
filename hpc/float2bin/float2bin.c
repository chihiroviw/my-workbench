#include <stdio.h>
#include <stdlib.h>

void print_float(float f){
	int len = 32;
	unsigned int fi = *(unsigned int *)(&f);
	for(int i=0; i<len; i++){
		printf("%d",(fi&(0x01<<i)) == 0 ? 0 : 1);
	}
	printf("\n");
}

int main(void)
{
  float f = 2.5f;

  printf("float=%16.8e\n",f);

  print_float(f);
  return 0;
}

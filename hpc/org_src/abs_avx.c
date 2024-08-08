#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>

#define SIZE    (3072)

float in[SIZE]   __attribute__((aligned(32)));
float out[SIZE]  __attribute__((aligned(32)));

int main(int argc, char **argv)
{
  int i,loop;
  int ret;
  __m256 *in_avx = (__m256 *)in;
  __m256 *out_avx = (__m256 *)out;
  __m256 tmp, pat, neg, zero = _mm256_set1_ps(0.0f);
  __m256 minusone = _mm256_set1_ps(-1.0f);

  for(i=0;i<SIZE;i+=2){
    in[i]=i+1;
    in[i+1]=-(i+2);
  }

  /* Calculate absolute values */
  for(loop=0;loop<1000000;loop++){
    for (i = 0; i < SIZE/8; i++) {
      // if in_avx[i] is larger than zero, pat becomes 1
      pat = _mm256_cmp_ps(in_avx[i],zero,_CMP_GT_OQ);

      // neg is in_avx[i] x minusone
      neg = _mm256_mul_ps(in_avx[i],minusone);

      // choose neg or in_avx[i] depending on pat
      // if pat==1 out_avx[i]=in_avx[i], else out_avx[i]=neg
      out_avx[i] = _mm256_blendv_ps(neg, in_avx[i], pat);
    }
  }

  for (i = SIZE-3; i < SIZE; i++) {
    printf("in [%02d]=%0.0f\n", i, in[i]);
    printf("out[%02d]=%0.0f\n", i, out[i]);
  }
  
  return 0;
}

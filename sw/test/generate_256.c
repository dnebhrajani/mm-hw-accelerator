#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

int main(void) {
  srand(time(NULL));
  int N = 16;
  
  printf("vars = %d;\n\n", N);

  printf("vector = [");
  for (int i = 0; i < N; ++i) {
    int16_t x = (rand() % 65536) - 32768;
    printf("%d", x);
    if (i != N-1) printf(",\n");
  }
  printf("];\n\n");

  printf("matrix = [");
  for (int i = 0; i < N * N; ++i) {
    int16_t x = (rand() % 65536) - 32768;
    printf("%d", x);
    if (i != N * N - 1) {
      printf(",\n");
    }
    
  }
  printf("];\n");

  return 0;
}

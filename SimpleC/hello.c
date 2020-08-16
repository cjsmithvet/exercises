#include <stdio.h>
#include <float.h>

int main () {
  int i = 1;
  long l = 1;
  char beep = 7; /* ASCII beep */

  printf("Ints and longs:\n");
  printf("Storage size of int: %d\n", (int)sizeof(int));
  printf("Storage size of long: %d\n", (int)sizeof(long));
  printf("i is an integer set to 1. int i: %d \n",
	 i/*, (long)i*/);
  printf("i is an integer set to 1. Casting it to long: %ld \n",
	 (long)i);
  printf("Storage size for float : %d \n", (int)sizeof(float));
  printf("Minimum float positive value: %E\n", FLT_MIN );
  printf("Maximum float positive value: %E\n", FLT_MAX );
  printf("Precision value: %d\n", FLT_DIG );

  printf("Beep: %c\n", beep);
  return 0;
}


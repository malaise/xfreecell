#include <stdlib.h>

const int M = 2147483647; // 2^31 - 1
const int a = 314159269;
static int x = 0;

void NSsrand(int num)
{
  x = a * num % M;
}

int NSrand()
{
  x = a * x % M;
  return abs(x);
}


/*
unsigned int rseed;

void NSsrand(int x)
{
  rseed = x;
}

unsigned int NSrand()
{
  rseed = (2531011 + (rseed * 214013)) % 2147483648;
  
  return (rseed / 65536) % 32768;
}
*/

#include <stdio.h>
#include <stdlib.h>

const int Club = 0;
const int Diamond = 1;
const int Heart = 2;
const int Spade = 3;

static void printCard(int);

int main(int argc, char* argv[])
{
  if (argc == 0) {
    fprintf(stderr, "error\n");
    return 1;
  }

  FILE* fp = fopen("small-list", "r");

  fseek(fp, atoi(argv[1]) * 52, SEEK_SET);

  char line[52];

  fread(line, sizeof(char), 52, fp);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 7; j++) 
      printCard(line[i*7+j]);
    putchar('\n');
  }

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 6; j++) 
      printCard(line[i*6+j+28]);
    putchar('\n');
  }

  return 0;
}

void printCard(int i)
{
#ifdef REAL
  int suit = (i - 1) % 4;

  switch (suit) {
  case Club:
    printf("%2dC ", (i - 1) / 4);
    return;
  case Diamond:
    printf("%2dD ", (i - 1) / 4);
    return;
  case Heart:
    printf("%2dH ", (i - 1) / 4);
    return;
  case Spade:
    printf("%2dS ", (i - 1) / 4);
    return;
  }
  fprintf(stderr, "ER ");
#else
  printf("%2d ", i);
#endif
}

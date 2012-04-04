#include <stdio.h>
#include <stdlib.h>

static void initialize();
static void write_result();

int card[8][7];
int deck[52];
FILE* fp;

int main()
{
  int i, j, k;
  int wLeft = 52;

  fp = fopen("list", "w+");

  for (i = 0; i < 32001; i++) {
    initialize();
    srand(i);

    fprintf(fp, "num: %d\n", i);
    for (j = 0; j < 52; j++) {
      k = rand() % wLeft;
      card[j%8][j/8] = deck[k];
      deck[k] = deck[--wLeft];
    }

    wLeft = 52;
    write_result();
  }

  fclose(fp);

  return 0;
}

void initialize()
{
  int i, j;

  for (i = 0; i < 8; i++)
    for (j = 0; j < 7; j++)
      card[i][j] = 0;

  for (i = 0; i < 52; i++)
    deck[i] = i + 1;
}

void write_result()
{
  int i, j;

  for (i = 0; i < 8; i++) {
    for (j = 0; j < 7; j++)
      fprintf(fp, "%d ", card[i][j]);
    fputc('\n', fp);
  }

  fputc('\n', fp);
}

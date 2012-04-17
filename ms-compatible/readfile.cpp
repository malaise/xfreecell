#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  if (argc != 2) {
    fprintf(stderr, "error\n");
    return 1;
  }

  FILE* fp = fopen("list", "r");
  fseek(fp, atoi(argv[1]), SEEK_SET);

  char line[100];

  for (int i = 0; i < 9; i++) {
    fgets(line, 100, fp);
    printf("%s", line);
  }

  return 0;
}

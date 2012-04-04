#include <stdio.h>

int main()
{
  FILE* out = fopen("small-list", "w+");
  FILE* in = fopen("list", "r");

  char line[100];
  char output[52];
  for (int i = 0; i < 32001; i++) {
    fgets(line, 100, in);
    for (int j = 0; j < 4; j++) {
      fgets(line, 100, in);
      sscanf(line, "%d %d %d %d %d %d %d", output+7*j, output+7*j+1, output+7*j+2,
	     output+7*j+3, output+7*j+4, output+7*j+5, output+7*j+6);
    }
    for (int j = 0; j < 4; j++) {
      fgets(line, 100, in);
      sscanf(line, "%d %d %d %d %d %d", output+6*j+28, output+6*j+29, output+6*j+30,
	     output+6*j+31, output+6*j+32, output+6*j+33);
    }
    fgets(line, 100, in);
    //    fprintf(out, "%s\n", output);
    fwrite(output, sizeof(char), 52, out);
  }

  return 0;
}

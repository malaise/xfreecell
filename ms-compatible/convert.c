#include <stdio.h>
#include <stdlib.h>

int main (void) {
  FILE *fi, *fo;
  char line[52];
  int i, j;
  size_t res;

  fi = fopen ("MSNumbers", "r");
  if (fi == NULL) {
    perror ("Fopen fi");
    exit (1);
  }
  fo = fopen ("MSNumbers.h", "w");
  if (fi == NULL) {
    perror ("Fopen fi");
    exit (1);
  }

  fprintf (fo, "static char MSNumbers[] = {\n");
  for (i = 1; i <= 32001; i++) {
    res = fread (line, 1, 52, fi);
    if (res != 52) {
      perror ("Fread fi");
      exit (1);
    }
    for (j = 0; j < 52; j++) {
      fprintf (fo, "0x%02x", line[j]);
      if (j != 51) fprintf (fo, ", ");
    }
    if (i != 32001) fprintf (fo, ",\n");
  }
  fprintf (fo, "};\n");

  fclose (fi);
  fclose (fo);

  exit (0);
}


#include <stdio.h>
#include <stdlib.h>

int square(int v) {
  return v * v;
}

void usage() {
  printf("Syntax: ./square <NUM>\n");
}

int main(int argc, char *argv[]) {
  // check enough args given, abort if not
  if (argc != 2) {
    usage();
    return EXIT_SUCCESS;
  }

  // parse out number from first argument string, argv[1], abort if it's not a
  // number
  int v;
  if (sscanf(argv[1], "%d", &v) != 1) {
    usage();
    return EXIT_SUCCESS;
  }

  int v_squared = square(v);
  printf("%d\n", v_squared);

  return EXIT_SUCCESS;
}

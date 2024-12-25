#include <stdlib.h>
#include <stdio.h>

/**
 * Basic welcome program - prompts for a name, then indicates that person is cool.
 */

int main(int argc, char* argv[]) {
    char name[64]; // buffer for the name, 64 characters long
    printf("Input name: "); // print prompt
    scanf("%63s",name); // read the name
    printf("%s is very cool!\n",name); // indicate that the so-named person is, in fact, cool
    return 0;
}


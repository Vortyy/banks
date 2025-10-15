#include <stdio.h>

#include "bank.h"

#define ARENA_IMPLEMENTATION
#include "arena.h"

Arena a = { 0 };

// TODO: full math +/- maybe * ???
// TODO: just clean print each month
// TODO: think about command to add them easily

int main(int argc, char *argv[]){
  printf("HELLO WORLDOOOO\n");

  arena_free(&a);

  return 0;
}

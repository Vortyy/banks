#include <stdio.h>
#include <stdlib.h>

#define MAX_EXPENSES 1000
#include "bank.h"

#define ARENA_IMPLEMENTATION
#include "arena.h"

Arena a = { 0 };
Account account;

// TODO: full math +/- maybe * ???
// TODO: just clean print each month
// TODO: think about command to add them easily

Currency read_currency(char * arg){
  char number[MAX_CURRENCY_N_SIZE + 1];
  char fraction[MAX_CURRENCY_F_SIZE + 1];

  char c;
  int i = 0;

  // Number part
  while((c = *arg++) != '.' && c != '\0' && i < MAX_CURRENCY_N_SIZE)
    number[i++] = c;
  number[i] = '\0';

  i = 0;

  while((c = *arg++) != '\0' && i < MAX_CURRENCY_F_SIZE)
    fraction[i++] = c;
  fraction[i] = '\0';

  return (Currency) {
    .number = atoi(number),
    .fraction = atoi(fraction)
  };
}

int main(int argc, char *argv[]){
  if(argc <= 1) {
    printf("ERROR: No argument given\n");
    return 1;
  }

  Currency c = read_currency(*++argv);
  Currency b = {.number = 20, .fraction=95 };

  print_currency(c);
  print_currency(b);

  add(&c, b);

  print_currency(c);

  arena_free(&a);
  return 0;
}

#include <stdio.h>

#define MAX_EXPENSES 1000
#include "bank.h"

#define ARENA_IMPLEMENTATION
#include "arena.h"

Arena a = { 0 };
Account account;

// TODO: full math +/- maybe * ???
// TODO: just clean print each month
// TODO: think about command to add them easily

int main(int argc, char *argv[]){
  account = (Account) {
    .list = (Expense *) arena_alloc(&a, sizeof(Expense) * MAX_EXPENSES),
    .exp_nb = 0,
    .name = "Yohan\0",
    .max_exp_nb = MAX_EXPENSES
  };

  printf("Account created: %s\n", account.name);
  arena_free(&a);
  return 0;
}

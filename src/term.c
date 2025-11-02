#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#define MAX_EXPENSES 1000
#include "bank.h"

#define PATH_STORAGE "./storage/my_test.csv"
#define PATH_MAX_SIZE 100

#define ARENA_IMPLEMENTATION
#include "arena.h"

Arena a = { 0 };
char path[PATH_MAX_SIZE]; 

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

  if(i != MAX_CURRENCY_F_SIZE)
    while(i < MAX_CURRENCY_F_SIZE)
      fraction[i++] = '0';
    
  fraction[i] = '\0';

  return (Currency) {
    .number = atoi(number),
    .fraction = atoi(fraction)
  };
}

void save_expense(Expense * exp){
  FILE * fp;

  struct tm * pTime = localtime(&exp->date);
  strftime(path, PATH_MAX_SIZE, PATH_STORAGE, pTime);

  if((fp = fopen(path, "a+")) == NULL){
    printf("ERROR: Unable to open '%s'\n", path);
  }

  // date, cost, author, type
  int ret = fprintf(fp, "%ld,%d.%02d,%s,%d\n", exp->date, exp->currency.number, exp->currency.fraction, exp->author, exp->type);

  fclose(fp);
} 

void print_help(){
  printf("Help: \n");
  printf("  bank add: \n");
}

// TODO: Add expense + income + add monthly recaps with stats
int main(int argc, char *argv[]){
  if(argc <= 1) {
    print_help();
    return 1;
  }

  char * current_arg = *++argv;
  if(!strcmp("--help", current_arg) || !strcmp("-h", current_arg)){
    print_help();
    return 1;
  }

  // term -e (+/-)154.59 "pizzas with my friends"
  if(strcmp("add", *argv) == 0 && argc == 4){
    argv++;
    Expense exp;

    if((*argv)[0] != '+' && (*argv)[0] != '-'){
      printf("ERROR: Unable to identify expense type\n");
      return 1;
    }

    exp.type = ((*argv)[0] == '+') ? INCOME : OUTCOME;
    exp.currency = read_currency((*argv) + 1); // Skip type
    exp.author = *++argv;
    exp.date = time(NULL);

    save_expense(&exp);
  }

  arena_free(&a);
  return 0;
}

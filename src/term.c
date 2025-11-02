#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define _XOPEN_SOURCE
#include <time.h>

#define MAX_EXPENSES 1000
#include "bank.h"

#define PATH_STORAGE "./storage/my_test.csv"
#define PATH_MAX_SIZE 100

#define ARENA_IMPLEMENTATION
#include "arena.h"

Arena a = { 0 };
char path[PATH_MAX_SIZE]; 

void exit_clean(){
  arena_free(&a);
}

int parse_date(char * arg, struct tm * date){
  char buffer[2];
  int i = 0;

  // day
  while(*arg != '/' && i < 2){
    buffer[i++] = *arg++;
  }

  if(*arg != '/')
    return 1;
  arg++;

  int day = atoi(buffer);
  if(day <= 0 || day > 31)
    return 1;

  // month
  buffer[0] = '\0';
  buffer[1] = '\0';
  i = 0;
  while(*arg != '\0' && i < 2)
    buffer[i++] = *arg++;

  if(*arg != '\0')
    return 1;

  int month = atoi(buffer);
  if(month <= 0 || month > 12)
    return 1;

  // set values
  date->tm_mday = day;
  date->tm_mon = month - 1; // [0, 11]

  return 0;
}

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

int save_expense(Expense * exp){
  FILE * fp;

  struct tm * pTime = localtime(&exp->date);
  strftime(path, PATH_MAX_SIZE, PATH_STORAGE, pTime);

  if((fp = fopen(path, "a+")) == NULL){
    printf("ERROR: Unable to open '%s'\n", path);
    return 1;
  }

  // date, cost, author, type
  int ret = fprintf(fp, "%ld,%d.%02d,%s,%d\n", exp->date, exp->currency.number, exp->currency.fraction, exp->author, exp->type);

  fclose(fp);
  return 0;
} 

void print_help(){
  printf("Help: \n");
  printf("  add: to add an expense\n");
}

// Add an expense
// bank add (+/-)154.59 "pizzas with my friends" -d 19/10
int cmd_add(int argc, char *argv[]){
    Expense exp;
    int opt, ret;
    time_t date = time(NULL);
    struct tm * date_opt = localtime(&date);

    argv++;
    if((*argv)[0] != '+' && (*argv)[0] != '-'){
      printf("ERROR: Unable to identify expense type\n");
      return EXIT_FAILURE;
    }

    opt = getopt(argc, argv, "d:");
    if (opt != -1 && opt == 'd'){
      if(parse_date(optarg, date_opt)){
        printf("ERROR: Unable to parse the given date '%s' format should be the following\n", optarg);
        return EXIT_FAILURE;
      }
      date = mktime(date_opt);
    }

    exp.type = ((*argv)[0] == '+') ? INCOME : OUTCOME;
    exp.currency = read_currency((*argv) + 1); // Skip type
    exp.author = *++argv;
    exp.date = date;

    ret = save_expense(&exp);

    if(ret == 0){
      print_exp(&exp);
    }

    return ret;
}

int cmd_monthly_resume(){
  printf("NOT IMPLEMENTED YET...\n");
  return 0;
}

int main(int argc, char *argv[]){
  atexit(exit_clean);
  int ret = 1;


  if(argc <= 1) {
    print_help();
    exit(EXIT_FAILURE);
  }

  argv++;
  if(!strcmp("--help", *argv) || !strcmp("-h", *argv)){
    print_help();
    exit(EXIT_SUCCESS);
  }

  // Parse cmd
  if(!strcmp("add", *argv) && argc >= 4){
    ret = cmd_add(argc, argv);
  }

  exit(ret);
}

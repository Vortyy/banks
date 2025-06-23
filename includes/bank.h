#ifndef BANK_H 
#define BANK_H

#include <time.h>
#include <string.h>
#include "arena.h"

extern Arena a; // WARNING: Declared inside displayer.c

typedef enum __expense_type {
  INCOME,
  OUTCOME,
  ERROR
} ExpenseType;

typedef struct __expense_struct {
  float cost;
  ExpenseType type;
  time_t date;
  char * author;

  char * s_cost;
  char * s_date;
} Expense;

typedef struct __account_struct {
  Expense * list;
  char * name;
  int exp_nb;
  int max_exp_nb;
} Account;

void exp_init(Expense * exp, float cost, time_t exp_time, ExpenseType type, char * author);
ExpenseType get_type(char * type_string);
float get_type_sign(ExpenseType type);

#define exp_init_now(exp, cost, type, author) exp_init(exp, cost, (time_t) NULL, type, author)
#define exp_print(exp) TraceLog(LOG_INFO, "%s: exp.cost: %d, exp.author: %s, exp.date %s", LOG_PNAME, exp->cost, exp->author, exp->sdate)

void account_add_exp(Account * account, float cost, time_t time, ExpenseType type, char * author);
float account_get_total(Account * account);

#endif

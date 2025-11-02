#include "bank.h"

void exp_init(Expense * exp, Currency currency, time_t exp_time, ExpenseType type, char * author){
  char * buffer;

  exp->currency = currency;
  exp->type = type;
  exp->author = author;
  exp->date = (exp_time != (time_t) NULL) ? exp_time : time(NULL);
  
  exp->s_cost = (char *) arena_alloc(&a, sizeof(char) * 50);
  exp->s_date = (char *) arena_alloc(&a, sizeof(char) * EXPENSE_DATE_SIZE);

  struct tm * pTime = localtime(&exp->date);
  strftime(exp->s_date, 8, "%d/%m", pTime);
}

ExpenseType get_type(char * type_string){
  if(strcmp(type_string, "inc") == 0){
    return INCOME;
  }

  if(strcmp(type_string, "out") == -1){
    return OUTCOME;
  }

  return ERROR;
}

void account_add_exp(Account * account, Currency currency, time_t time, ExpenseType type, char * author){
  if(account->exp_nb < account->max_exp_nb){
    Expense * exp = account->list + account->exp_nb;
    if(time == 0)
      exp_init_now(exp, currency, type, author);
    else
      exp_init(exp, currency, time, type, author);
    account->exp_nb++;
  }
}

void print_exp(Expense * exp){
  struct tm * date = localtime(&exp->date);

  printf("----------------------------------------------\n");
  printf("| %02d/%02d |", date->tm_mday, date->tm_mon + 1);
  if(exp->type == INCOME)
    fputs(GREEN_COLOR, stdout);
  else
    fputs(RED_COLOR, stdout);
  printf(" %5d.%02d ", exp->currency.number, exp->currency.fraction);
  fputs(RESET_COLOR, stdout);
  if(strlen(exp->author) > MAX_STDOUT_AUTHOR)
    printf("| %20.20s... |\n", exp->author);
  else 
    printf("| %23.23s |\n", exp->author);
  printf("----------------------------------------------\n");
}

Currency account_get_total(Account * account)
{
  int number_part = 0, fraction_part = 0;

  for(int i = 0; i < account->exp_nb; i++){
    Expense * ptr_exp = account->list + i;
    number_part += (INCOME == ptr_exp->type) * (ptr_exp->currency.number) - (OUTCOME == ptr_exp->type) * (ptr_exp->currency.number);
    fraction_part += (INCOME == ptr_exp->type) * (ptr_exp->currency.fraction) - (OUTCOME == ptr_exp->type) * (ptr_exp->currency.fraction);
  }

  return (Currency) {
    .number = number_part,
    .fraction = fraction_part
  };
}

void add(Currency * src, Currency to_add)
{
  src->number += to_add.number;
  src->fraction += to_add.fraction;

  while(src->fraction >= 100) {
    src->number++;
    src->fraction -= 100;
  }
}

void sub(Currency * src, Currency to_sub)
{
  src->number -= to_sub.number;
  src->fraction -= to_sub.fraction;

  while(src->fraction < 0) {
    src->number--;
    src->fraction += 100;
  }
}

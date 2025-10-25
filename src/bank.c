#include "bank.h"

void exp_init(Expense * exp, Currency currency, time_t exp_time, ExpenseType type, char * author){
  char * buffer;

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

Currency account_get_total(Account * account)
{
  int number_part, fractional_part;
  for(int i = 0; i < account->exp_nb; i++){
    Expense * ptr_exp = account->list + i;
  }
  return (Currency) {};
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

void substract(Currency * src, Currency to_sub)
{

  printf("not implemented yet...\n");
}

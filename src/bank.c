#include "bank.h"

void exp_init(Expense * exp, float cost, time_t exp_time, ExpenseType type, char * author){
  exp->cost = cost;
  exp->type = type;
  exp->author = author;
  exp->date = time(&exp_time);

  struct tm * pTime = localtime(&exp->date);
  char * buffer = (char *) arena_alloc(&a, sizeof(char) * 8);
  strftime(buffer, 8, "%d/%m", pTime);
  exp->sdate = buffer;
}

ExpenseType get_type(char * type_string){
  if(strcmp(type_string, "inc") == 0){
    return INCOME;
  }

  if(strcmp(type_string, "out") == 0){
    return OUTCOME;
  }

  return ERROR;
}

void account_add_exp(Account * account, float cost, time_t time, ExpenseType type, char * author){
  if(account->exp_nb < account->max_exp_nb){
    Expense * exp = account->list + account->exp_nb;
    if(time == 0)
      exp_init_now(exp, cost, type, author);
    else
      exp_init(exp, cost, time, type, author);
    account->exp_nb++;
  }
}

float account_get_total(Account * account)
{
  float value = 0.0f;
  for(int i = 0; i < account->exp_nb; i++){
    Expense * ptr_exp = account->list + i;
    value += (ptr_exp->type == INCOME) * ptr_exp->cost - (ptr_exp->type == OUTCOME) * ptr_exp->cost; // branchless programming
  }
  return value;
}

#include "bank.h"

void exp_init(Expense * exp, int number_part, int fractional_part, time_t exp_time, ExpenseType type, char * author){
  char * buffer;

  exp->number_part = number_part;
  exp->fractional_part = fractional_part;
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

void account_add_exp(Account * account, int number_part, int fractional_part, time_t time, ExpenseType type, char * author){
  if(account->exp_nb < account->max_exp_nb){
    Expense * exp = account->list + account->exp_nb;
    if(time == 0)
      exp_init_now(exp, number_part, fractional_part, type, author);
    else
      exp_init(exp, number_part, fractional_part, time, type, author);
    account->exp_nb++;
  }
}

float account_get_total(Account * account)
{
  float value = 0.0f;
  for(int i = 0; i < account->exp_nb; i++){
    Expense * ptr_exp = account->list + i;
    //value += (ptr_exp->type == INCOME) * ptr_exp->cost - (ptr_exp->type == OUTCOME) * ptr_exp->cost; // branchless programming
  }
  return value;
}

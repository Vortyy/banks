#include "bank.h"

void exp_init(Expense * exp, int price, time_t exp_time, ExpenseType type, char * author){
  char * buffer;

  exp->price = price;
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

void account_add_exp(Account * account, int price, time_t time, ExpenseType type, char * author){
  if(account->exp_nb < account->max_exp_nb){
    Expense * exp = account->list + account->exp_nb;
    if(time == 0)
      exp_init_now(exp, price, type, author);
    else
      exp_init(exp, price, time, type, author);
    account->exp_nb++;
  }
}

void acc_add(Account * account, Expense expense){
  if(account->exp_nb < account->max_exp_nb){
    account->total += (expense.type == INCOME) * expense.price - (expense.type != INCOME) * expense.price;
    account->list[account->exp_nb] = expense;
    account->exp_nb++;
  }
}

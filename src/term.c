#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <wchar.h>

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
char * prog;
time_t current_time;

Expense list[MAX_EXPENSES];
Account month_account = {
  .total = { 0, 0 },
  .name = "Yohan",
  .list = list,
  .exp_nb = 0,
  .max_exp_nb = MAX_EXPENSES
};

char * headers[] = {"date", "price", "author"};
int exp_col_w[3] = {
  5, // date
  8, // price
  20 // author
};

// Renderer
#include <assert.h>

#define TOP_L_CORNER "\u256D"
#define TOP_R_CORNER "\u256E"
#define BOT_L_CORNER "\u2570"
#define BOT_R_CORNER "\u256F"
#define HOR_LINE "\u2500"
#define VER_LINE "\u2502"

#define CROSS "\u253C"
#define L_CROSS "\u251C"
#define R_CROSS "\u2524"

#define RED_COLOR "\x1b[31m"
#define GREEN_COLOR "\x1b[32m"
#define RESET_COLOR "\x1b[0m"

#define DEFAULT_PADDING 1

#define MAX_TABLE_HEADER 10
#define MAX_TABLE_CONTENT 1000

typedef struct term_table {
  int w;
  int pad;

  int nb_row;
  int nb_col;

  int * col_w;

  char ** headers;
  char * content[MAX_TABLE_CONTENT];
} Table;

Table table_create(int * col_w, int nb_col, char ** headers){
  assert(nb_col < MAX_TABLE_HEADER);

  int w = 0;
  for(int i = 0; i < nb_col; i++)
    w += col_w[i];

  w += nb_col - 1;
  w += DEFAULT_PADDING * 2 * nb_col;
  
  return (Table) {
    .w = w,
    .pad = DEFAULT_PADDING,
    .nb_row = 0,
    .nb_col = nb_col,
    .col_w = col_w,
    .headers = headers
  };
}

void print_table(Table * table){
  int i, j;
  fputs(TOP_L_CORNER, stdout);
  for(i = 0; i < table->w; i++)
    fputs(HOR_LINE, stdout);
  fputs(TOP_R_CORNER, stdout);
  putc('\n', stdout);

  if(table->headers != NULL) {
    fputs(VER_LINE, stdout);
    for(j = 0; j < table->nb_col; j++){
      int hsize = strlen(table->headers[j]);
      if(hsize < table->col_w[j]){
        int blank = table->col_w[j] - hsize + 1;
        while(blank-- > 0) putc(' ', stdout);
        printf("%s ", table->headers[j]);
      } else {
        printf("TODO");
      }
      fputs(VER_LINE, stdout);
    }
    putc('\n', stdout);

    fputs(L_CROSS, stdout);
    for(j = 0; j < table->nb_col; j++){
      for(i = 0; i < table->col_w[j] + DEFAULT_PADDING * 2; i++){
        fputs(HOR_LINE, stdout);
      }

      if(j != table->nb_col - 1)
        fputs(CROSS, stdout);
    }
    fputs(R_CROSS, stdout);
    putc('\n', stdout);
  }

  for(i = 0; i < table->nb_row; i++){
    fputs(VER_LINE, stdout);
    for(j = 0; j < table->nb_col; j++){
      // TODO: if(strlen(exp->author) > MAX_STDOUT_AUTHOR)
      printf(" %s ", table->content[i * table->nb_col + j]);
      fputs(VER_LINE, stdout);
    }
    putc('\n', stdout);
  }

  fputs(BOT_L_CORNER, stdout);
  for(i = 0; i < table->w; i++)
    fputs(HOR_LINE, stdout);
  fputs(BOT_R_CORNER, stdout);
  putc('\n', stdout);
}

void add_row(Table * self, int n, ...){
  assert(n == self->nb_col);

  va_list ap;
  va_start(ap, n);
  int start_idx = self->nb_col * self->nb_row;

  for(int i = 0; i < self->nb_col; i++){
    char * value = va_arg(ap, char *);
    self->content[start_idx + i] = value;
  }

  va_end(ap);
  self->nb_row++;
}

// Renderer end

void exit_clean(){
  arena_free(&a);
}

void print_exp(Expense * exp){
  struct tm * date = localtime(&exp->date);
  Table table = table_create(exp_col_w, 3, headers);

  char date_buf[20];
  char price_buf[20];
  char author_buf[20];

  sprintf(date_buf, "%02d/%02d", date->tm_mday, date->tm_mon + 1);
  if(exp->type == INCOME)
    sprintf(price_buf, GREEN_COLOR"%5d.%02d"RESET_COLOR, exp->currency.number, exp->currency.fraction);
  else
    sprintf(price_buf, RED_COLOR"%5d.%02d"RESET_COLOR, exp->currency.number, exp->currency.fraction);
  sprintf(author_buf, "%10s", exp->author);

  add_row(&table, 3, date_buf, price_buf, author_buf);

  print_table(&table);
}

int get_line(char * buffer, int buffer_size, FILE * stream){
  int c = 0, i = 0;
  while((c = fgetc(stream)) != EOF && c != '\n'){
    if(i > buffer_size - 2){ // Size - 1 (- 1 '\0')
      fprintf(stderr, "%s: error overflow buffer\n", prog);
      return -1;
    }

    buffer[i++] = c;
  }

  buffer[i] = '\0';
  return c;
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
  printf("  resume: to resume monthly expense\n");
}

// Add an expense
// bank add (+/-)154.59 "pizzas with my friends" -d 19/10
int cmd_add(int argc, char *argv[]){
    Expense exp;
    int opt, ret;
    time_t date = current_time;
    struct tm * date_opt = localtime(&current_time);

    argv++;
    if((*argv)[0] != '+' && (*argv)[0] != '-'){
      printf("ERROR: Unable to identify expense type\n");
      return EXIT_FAILURE;
    }

    opt = getopt(argc - 2, argv, "d::");
    if (opt == 'd'){
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

char * get_next_token(char * from, char * buffer, char delimiter){
  int i = 0;
  while(*from != delimiter && *from != '\n' && *from != EOF){
    buffer[i] = *from;
    i++;
    from++;
  }
  buffer[i] = '\0';

  if(*from == delimiter)
    from++;

  return from;
};

void fill_tab(Table * table, Account * account){
  for(int i = 0; i < account->exp_nb; i++){
    Expense exp = account->list[i];
    struct tm * time = localtime(&exp.date);

    char * date_str = (char *) arena_alloc(&a, sizeof(char) * 6);
    char * price_str = (char *) arena_alloc(&a, sizeof(char) * 20);

    sprintf(date_str, "%02d/%02d", time->tm_mday, time->tm_mon + 1);
    if(exp.type == INCOME)
      sprintf(price_str, GREEN_COLOR"%5d.%02d"RESET_COLOR, exp.currency.number, exp.currency.fraction);
    else
      sprintf(price_str, RED_COLOR"%5d.%02d"RESET_COLOR, exp.currency.number, exp.currency.fraction);

    add_row(table, 3, date_str, price_str, exp.author);
  }
}

int cmd_monthly_resume(int argc, char *argv[]){
  struct tm * readable_time = localtime(&current_time);
  int current_mon = readable_time->tm_mon;
  int ret = 1;

  FILE * fp;
  if((fp = fopen(PATH_STORAGE, "r")) == NULL){
    fprintf(stderr, "%s: error while loading '%s' file", prog, PATH_STORAGE);
    return 1;
  }

  char line[BUFSIZ];
  char token[100];

  while((ret = get_line(line, BUFSIZ, fp)) != EOF && ret != -1){
    Expense current_exp;

    // Date
    char * line_ptr = get_next_token(line, token, ',');
    current_exp.date = (time_t) atol(token);
    struct tm * time = localtime(&current_exp.date);

    if(time->tm_mon != current_mon)
      continue;

    // Price
    line_ptr = get_next_token(line_ptr, token, ',');
    current_exp.currency = read_currency(token);

    // Author
    line_ptr = get_next_token(line_ptr, token, ',');
    current_exp.author = arena_alloc(&a, sizeof(char) * 21);
    sprintf(current_exp.author, "%20.20s", token);

    // Type
    line_ptr = get_next_token(line_ptr, token, ',');
    current_exp.type = (token[0] == '1') ? OUTCOME : INCOME;

    acc_add(&month_account, current_exp);
  }

  Table table = table_create(exp_col_w, 3, headers);
  fill_tab(&table, &month_account);
  print_table(&table);

  printf("\nResult for this month are: %4d.%02d \n", month_account.total.number, month_account.total.fraction);

  return 0;
}

int main(int argc, char *argv[]){
  atexit(exit_clean);
  int ret = 1;

  current_time = time(NULL);

  if(current_time == -1){
    fprintf(stderr, "%s: Unable to get current time\n", prog);
    exit(EXIT_FAILURE);
  }

  prog = *argv++;
  if(argc <= 1 || !strcmp("--help", *argv) || !strcmp("-h", *argv)) {
    print_help();
    exit(EXIT_SUCCESS);
  }

  // Parse cmd
  if(!strcmp("add", *argv) && argc >= 4){
    ret = cmd_add(argc, argv);
  } else if (!strcmp("resume", *argv)){
    ret = cmd_monthly_resume(argc, argv);
  }

  return ret;
}

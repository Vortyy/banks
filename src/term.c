#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define _XOPEN_SOURCE
#include <time.h>

#define MAX_EXPENSES 1000
#include "bank.h"

// TODO
#ifdef DEBUG
#define PATH_STORAGE "./storage/my_test.csv"
#endif

#define STORAGE_PATH_ENV "BANK_STORAGE_PATH"
#define PATH_MAX_SIZE 100
#define BUFFER_STR_SIZE 100

#define DRAW_LINE(table)          \
  putc(' ', stdout);              \
  for(int i = 0; i < table.w; i++)\
    fputs("\u2500", stdout);      \
  putc('\n', stdout);

#define ARENA_IMPLEMENTATION
#include "arena.h"

Arena a = { 0 };
char * prog;
char * storage_path;
time_t current_time;

char * headers[] = {"date", "price", "author"};
int exp_col_w[3] = {
  5, // date
  8, // price
  20 // author
};

Expense list[MAX_EXPENSES];
Account month_account = {
  .total = 0,
  .name = "Yohan",
  .list = list,
  .exp_nb = 0,
  .max_exp_nb = MAX_EXPENSES
};

// Renderer
#include <assert.h>

// Symbols
#define TOP_L_CORNER "\u256D"
#define TOP_R_CORNER "\u256E"
#define BOT_L_CORNER "\u2570"
#define BOT_R_CORNER "\u256F"
#define HOR_LINE "\u2500"
#define VER_LINE "\u2502"

#define CROSS "\u253C"
#define L_CROSS "\u251C"
#define R_CROSS "\u2524"

// Colors 
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
  char author_buf[100];

  sprintf(date_buf, "%02d/%02d", date->tm_mday, date->tm_mon + 1);
  if(exp->type == INCOME)
    sprintf(price_buf, GREEN_COLOR"%5d.%02d"RESET_COLOR, get_number(exp->price), get_fraction(exp->price));
  else
    sprintf(price_buf, RED_COLOR"%5d.%02d"RESET_COLOR, get_number(exp->price), get_fraction(exp->price));
  sprintf(author_buf, "%20.20s", exp->author);

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

int read_currency(char * arg){
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

  int n = atoi(number) * 100;
  int f = atoi(fraction);
  return n + f;
}

int save_expense(Expense * exp){
  FILE * fp;

  if((fp = fopen(storage_path, "a+")) == NULL){
    printf("ERROR: Unable to open '%s'\n", storage_path);
    return 1;
  }

  // date, cost, author, type
  int ret = fprintf(fp, "%ld,%d.%02d,%s,%d\n", exp->date, get_number(exp->price), get_fraction(exp->price), exp->author, exp->type);

  fclose(fp);
  return 0;
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
    exp.price = read_currency((*argv) + 1); // Skip type
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
      sprintf(price_str, GREEN_COLOR"%5d.%02d"RESET_COLOR, get_number(exp.price), get_fraction(exp.price));
    else
      sprintf(price_str, RED_COLOR"%5d.%02d"RESET_COLOR, get_number(exp.price), get_fraction(exp.price));

    add_row(table, 3, date_str, price_str, exp.author);
  }
}

int cmd_monthly_resume(int argc, char *argv[]){
  struct tm * readable_time = localtime(&current_time);
  int current_mon = readable_time->tm_mon;
  int ret = 1;

  FILE * fp;
  if((fp = fopen(storage_path, "r")) == NULL){
    fprintf(stderr, "%s: error while loading '%s' file", prog, storage_path);
    return 1;
  }

  char * author_filter = NULL;
  int opt = getopt(argc - 1, argv, "a:");
  if(opt == 'a')
    author_filter = optarg;

  char line[BUFSIZ];
  char token[BUFFER_STR_SIZE];

  while((ret = get_line(line, BUFSIZ, fp)) != EOF && ret != -1){
    Expense current_exp;

    // Date
    char * line_ptr = get_next_token(line, token, ',');
    current_exp.date = (time_t) atol(token);
    readable_time = localtime(&current_exp.date);

    if(readable_time->tm_mon != current_mon)
      continue;

    // Price
    line_ptr = get_next_token(line_ptr, token, ',');
    current_exp.price = read_currency(token);

    // Author
    line_ptr = get_next_token(line_ptr, token, ',');
    current_exp.author = arena_alloc(&a, sizeof(char) * 21);
    sprintf(current_exp.author, "%20.20s", token);

    if(author_filter != NULL && strcmp(token, author_filter))
      continue;

    // Type
    line_ptr = get_next_token(line_ptr, token, ',');
    current_exp.type = (token[0] == '1') ? OUTCOME : INCOME;

    acc_add(&month_account, current_exp);
  }

  Table table = table_create(exp_col_w, 3, headers);

  char title_str[BUFFER_STR_SIZE]; 
  readable_time = localtime(&current_time);
  strftime(title_str, BUFFER_STR_SIZE, "Summary for %B - %Y\n", readable_time);
  int space = (table.w/2 + 2) - (strlen(title_str) / 2);
  DRAW_LINE(table);
  for(int i = 0; i < space; i++)
    putc(' ', stdout);
  fputs(title_str, stdout);
  DRAW_LINE(table);

  fill_tab(&table, &month_account);
  print_table(&table);

  DRAW_LINE(table);
  space = (table.w/2 + 2) - (strlen("Result: %5d.%02d\n") / 2);
  for(int i = 0; i < space; i++)
    putc(' ', stdout);
  printf("Result: %5d.%02d\n", get_number(month_account.total), get_fraction(month_account.total));
  DRAW_LINE(table);

  return 0;
}

void print_help(){
  puts("Bank terminal (Bankt) -- Version 1.0.0");
  puts(" A program to manage my bank accounts locally with a nice TUI\n");

  puts("\x1b[1mCOMMANDS\x1b[0m");
  puts("│ add: to add an expense");
  puts("│ resume: to resume monthly expense\n");

  puts("To get commands usage and help you can type:");
  puts(" -> bankt commands -h ");
  puts(" -> bankt commands --help ");
}

int main(int argc, char *argv[]){
  atexit(exit_clean);
  int ret;

  current_time = time(NULL);
  if(current_time == -1){
    fprintf(stderr, "%s: Unable to get current time\n", prog);
    exit(EXIT_FAILURE);
  }

  prog = *argv++;

#ifdef DEBUG
  storage_path = PATH_STORAGE;
#else
  char * env = getenv(STORAGE_PATH_ENV);
  if(env == NULL){
    fprintf(stderr, "%s: env var '%s' not set properly\n", prog, STORAGE_PATH_ENV);
    exit(EXIT_FAILURE);
  }
  storage_path = env;
#endif

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

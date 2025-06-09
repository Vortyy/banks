#include <stdio.h>
#include <string.h>
#include <raylib.h>
#include <ctype.h>
#include <time.h>

// TODO: Maybe created a shared context to save data during runner exec
// Late binding can be fun -> https://stackoverflow.com/questions/32347910/oop-in-c-implicitly-pass-self-as-parameter

// Thx Tsoding and al.
#define ARENA_IMPLEMENTATION
#include "./includes/arena.h"

#define INPUT_SIZE 3 
#define DISPLAYED_COLUMN 3
#define SCREEN_MARGIN 10
#define TEXT_SIZE 20
#define FRM_COUNTER_MAX_VALUE 2000000
#define LOG_PNAME "MY_DISPLAYER"
#define PATH_STORAGE "./store.csv"

#define MAX_EXPENSES 100
#define CSV_DELIMITER ","

#define SET_CENTERED(box_size, elemnt_size) \
  ((box_size / 2) - (elemnt_size / 2))


/* Arena allocator */
Arena a = {0};

/* Input (24bytes) */ 
typedef struct __input_struct {
  char * content;
  const char * placeholder;
  int char_limit; 
  int current_pos;
} Input;

void input_init(Input * input, int char_limit, const char * placeholder);
void input_add_char(Input * self, char c);
void input_del_char(Input * self);
void input_reset(Input * self);

Input inputs[INPUT_SIZE];

int selected = 0;
int rect_prop[3]  = {10, 150, 30};

void input_init(Input * input, int char_limit, const char * placeholder){
  input->content = (char *) arena_alloc(&a, sizeof(char) * (char_limit + 1));
  input->placeholder = placeholder;
  input->char_limit = char_limit;

  *input->content = '\0';
  input->current_pos = 0;
} 

void input_add_char(Input * self, char c){
  if(self->current_pos < self->char_limit){
    self->content[self->current_pos] = c;
    self->content[self->current_pos + 1] = '\0';
    self->current_pos++;
  }
}

void input_del_char(Input * self){
  self->current_pos--;
  if(self->current_pos < 0) self->current_pos = 0;
  self->content[self->current_pos] = '\0';
}

void input_reset(Input * self){
  self->content[0] = '\0';
  self->current_pos = 0;
}

/* Account && Expenses */

typedef enum __expense_type {
  INCOME,
  OUTCOME,
  ERROR
} ExpenseType;

typedef struct __expense_struct {
  int cost;
  ExpenseType type;
  time_t date;
  char * sdate;
  char * author;
} Expense;

void save_expense(Expense * exp);


#define exp_init_now(exp, cost, type, author) exp_init(exp, cost, (time_t) NULL, type, author)

void exp_init(Expense * exp, int cost, time_t exp_time, ExpenseType type, char * author){
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

#define exp_print(exp) TraceLog(LOG_INFO, "%s: exp.cost: %d, exp.author: %s, exp.date %s", LOG_PNAME, exp->cost, exp->author, exp->sdate)

typedef struct __account_struct {
  Expense * list;
  char * name;
  int exp_nb;
  int max_exp_nb;
} Account;

void account_add_exp(Account * account, int cost, time_t time, ExpenseType type, char * author){
  if(account->exp_nb < account->max_exp_nb){
    Expense * exp = account->list + account->exp_nb;
    if(time == 0)
      exp_init_now(exp, cost, type, author);
    else
      exp_init(exp, cost, time, type, author);
    account->exp_nb++;
  }
}

Account account;

/* Window */
int frm_counter = 0;
int w = 0;
int h = 0;

/* Utils */
int store_inputs(){
  /* if an inputs is empty */
  if(inputs[0].current_pos == 0 || inputs[1].current_pos == 0 || inputs[2].current_pos == 0)
    return 1;

  int cost = atoi(inputs[0].content);
  if(cost == 0) // can't convert the cost to int
    return 1;

  ExpenseType type = get_type(inputs[1].content); 
  if(type == ERROR) // can't find the type
    return 1;

  char * author = strdup(inputs[2].content);

  account_add_exp(&account, cost, (time_t) 0, type, author);

  Expense test;
  exp_init_now(&test,  cost, type, author);
  save_expense(&test);
  
  return 0;
}

void reset_inputs(){
  for(int i = 0; i < INPUT_SIZE; i++){
    input_reset(&inputs[i]);
  }
}

void save_expense(Expense * exp){
  FILE * fp;

  if((fp = fopen(PATH_STORAGE, "a+")) == NULL){
    TraceLog(LOG_ERROR, "%s: unable to open filepath %s", LOG_PNAME, PATH_STORAGE);
  }

  // date, cost, author, type
  int ret = fprintf(fp, "%ld,%d,%s,%d\n", exp->date, exp->cost, exp->author, exp->type);

  fclose(fp);
} 

void load_storage(Account * account){ 
  int cost, type;
  time_t date;
  char * author;

  FILE * fp;

  if((fp = fopen(PATH_STORAGE, "r")) == NULL){
    TraceLog(LOG_ERROR, "%s: unable to open filepath %s", LOG_PNAME, PATH_STORAGE);
  }

  char buffer[BUFSIZ];
  // date, cost, author, type
  while(fgets(buffer, BUFSIZ, fp) != NULL){
    date = (time_t) atol(strtok(buffer, ","));
    cost = atoi(strtok(NULL, ","));
    author = strdup(strtok(NULL, ","));
    type = (ExpenseType) atoi(strtok(NULL, ","));

    account_add_exp(account, cost, date, type, author);
  }
   
  fclose(fp);
} 

// Call once when the lib is opened
void init()
{
  input_init(&inputs[0], 5, "Cost");
  input_init(&inputs[1], 3, "Type");
  input_init(&inputs[2], 10, "Author");

  account = (Account) {
    .list = (Expense *) arena_alloc(&a, sizeof(Expense) * MAX_EXPENSES),
    .exp_nb = 0,
    .name = "Yohan",
    .max_exp_nb = MAX_EXPENSES
  };

  load_storage(&account);
}

// Call once when the lib is closed
void clear()
{
  TraceLog(LOG_INFO, "%s: Clearing memory", LOG_PNAME);
  arena_free(&a);
}

// Call until the lib is closed/reloaded
void display()
{
  // UPDATE
  w = GetScreenWidth();
  h = GetScreenHeight();
  
  // Put a limit to add footer and + moving screen fucktin

  if(IsKeyPressed(KEY_TAB)){
    frm_counter = 0;
    selected++;
    if(selected >= INPUT_SIZE) selected = 0;
  }

  if(IsKeyPressed(KEY_BACKSPACE)){
    input_del_char(&inputs[selected]);
  }

  if(IsKeyPressed(KEY_ENTER)){
    if(store_inputs() == 0)
      reset_inputs();
  }

  int key = GetCharPressed();
  while (key > 0){
    if(isalnum(key)){
      input_add_char(&inputs[selected], key);
    }
    key = GetCharPressed();
  }

  frm_counter++;
  if(frm_counter > FRM_COUNTER_MAX_VALUE){
    frm_counter = 0;
  }

  // DRAWING
  BeginDrawing();
    ClearBackground(RAYWHITE);

    for(int i = 0; i < INPUT_SIZE; i++){
      int x = 10 + 160 * i;
      DrawRectangle(x, rect_prop[0], rect_prop[1], rect_prop[2], DARKGRAY);

      if(inputs[i].current_pos == 0){
        DrawText(inputs[i].placeholder, x + 5, rect_prop[0] + 5, TEXT_SIZE, GRAY);
      } else {
        DrawText(inputs[i].content, x + 5, rect_prop[0] + 5, TEXT_SIZE, RED);
      }

      if(selected == i){
        DrawRectangleLines(x, rect_prop[0], rect_prop[1], rect_prop[2], RED);
        if(inputs[i].current_pos < inputs[i].char_limit && inputs[i].current_pos > 0){
            if((frm_counter/20) % 2 == 0) DrawText("_", x + 8 + MeasureText(inputs[i].content, TEXT_SIZE), rect_prop[0] + 5, TEXT_SIZE, RED);
        }
      } else {
        DrawRectangleLines(x, rect_prop[0], rect_prop[1], rect_prop[2], BLACK);
      }
    }

    int box_size = ((w/2 - SCREEN_MARGIN * 2) / DISPLAYED_COLUMN);

    int y = 65;
    // Expenses list
    for(int i = 0; i < account.exp_nb && (y + 25) < h - 40; i++){
      Expense * ptr_exp = account.list + ((account.exp_nb - 1) - i);

      Color bg_color = (ptr_exp->type == INCOME) ? GREEN : RED;
      DrawRectangle(10, 60 + 30 * i, w/2 - 21,  30, bg_color);

      // Text measurement
      int author_size = MeasureText(ptr_exp->author, TEXT_SIZE);
      int cost_size = MeasureText(TextFormat("%d", ptr_exp->cost), TEXT_SIZE);
      int date_size = MeasureText(ptr_exp->sdate, TEXT_SIZE);

      // Write text
      // TODO: check if box_size > measure text and write a substring ???
      DrawText(ptr_exp->sdate, SET_CENTERED(box_size, date_size) + SCREEN_MARGIN, y, TEXT_SIZE, BLACK);//Date
      DrawText(ptr_exp->author, SET_CENTERED(box_size, author_size) + (box_size + SCREEN_MARGIN), y, TEXT_SIZE, BLACK);//Author
      DrawText(TextFormat("%d", ptr_exp->cost), SET_CENTERED(box_size, cost_size) + (box_size * 2 + SCREEN_MARGIN), y, TEXT_SIZE, BLACK);//Cost

      y += 30; // starting size need to depend on text size
    }

    // Draw expenses list container
    DrawRectangleLines(10, 60, w/2 - 20, h - 100 , BLACK);

    // Draw 3 lines + headers
    for(int i = 1; i < DISPLAYED_COLUMN; i++){
      int x = box_size * i + 10;
      DrawLine(x, 60, x, h - 40, BLACK);
    }

    // TODO: DrawLine and values each month on 6 last months
    // TODO: HashTable to get each month as linear array of values sorted by their time

  EndDrawing();
}

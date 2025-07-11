#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <raylib.h>
#include <ctype.h>
#include <sys/stat.h>

#include "bank.h" // Expenses && Account
#include "input.h" // Custom Input
#include "listview.h" // Custom Listview

// TODO: Make sorting
// TODO: Maybe created a shared context to save data during runner exec
// Late binding can be fun -> https://stackoverflow.com/questions/32347910/oop-in-c-implicitly-pass-self-as-parameter

// Thx Tsoding and al.
#define ARENA_IMPLEMENTATION
#include "arena.h"

#define INPUT_SIZE 3 
#define DISPLAYED_COLUMN 3
#define SCREEN_MARGIN 10
#define TEXT_SIZE 20
#define FRM_COUNTER_MAX_VALUE 2000000
#define LOG_PNAME "MY_DISPLAYER"

#define PATH_STORAGE "./storage/%m_%Y.csv"
#define PATH_MAX_SIZE 100

#define MAX_EXPENSES 1000
#define CSV_DELIMITER ","

#define VAMPIREDARK CLITERAL(Color){13, 2, 8, 255}

#define SET_CENTERED(box_size, elemnt_size) \
  ((box_size / 2) - (elemnt_size / 2))

void reset_inputs();
int store_inputs();
void save_expense(Expense * exp);
void load_storage(Account * account);
void read_file(FILE * fp, Account * account);
void filltab(Listview * v);

/* Arena allocator declaration */
Arena a = {0};

/* Window && Globals variables */
int frm_counter = 0;
int w = 0;
int h = 0;

Account account;
Input inputs[INPUT_SIZE];
Listview listview;

float current_total = 0.0f;

int selected = 0;
int rect_prop[3]  = {10, 150, 30};
int lv_row = 0;

char displayed_month[12];
char path[PATH_MAX_SIZE];

char * headers[] = {"Date", "Author", "Cost"};

/* Utils */
int store_inputs(){
  /* if an inputs is empty */
  if(inputs[0].current_pos == 0 || inputs[1].current_pos == 0 || inputs[2].current_pos == 0)
    return 1;

  float cost = atof(inputs[0].content);
  if(cost == 0) // can't convert the cost to int
    return 1;

  ExpenseType type = get_type(inputs[1].content); 
  if(type == ERROR) // can't find the type
    return 1;

  char * author = strdup(inputs[2].content);

  account_add_exp(&account, cost, (time_t) 0, type, author);

  Expense * exp_added = account.list + account.exp_nb - 1;
  save_expense(exp_added);

  lv_add_row(&listview, exp_added->s_date, exp_added->author, exp_added->s_cost);
  current_total += get_type_sign(exp_added->type) * exp_added->cost;
  
  return 0;
}

void reset_inputs(){
  for(int i = 0; i < INPUT_SIZE; i++){
    input_reset(&inputs[i]);
  }
  
  inputs[selected].state = 0;
  selected=0;
  inputs[selected].state = IS_SELECTED;
}

void save_expense(Expense * exp){
  FILE * fp;

  struct tm * pTime = localtime(&exp->date);
  strftime(path, PATH_MAX_SIZE, PATH_STORAGE, pTime);

  if((fp = fopen(path, "a+")) == NULL){
    TraceLog(LOG_ERROR, "%s: unable to open filepath %s", LOG_PNAME, path);
  }

  // date, cost, author, type
  int ret = fprintf(fp, "%ld,%6.2f,%s,%d\n", exp->date, exp->cost, exp->author, exp->type);

  fclose(fp);
} 

void read_file(FILE * fp, Account * account){
  float cost;
  int type;
  time_t date;
  char * author;

  char buffer[BUFSIZ];
  // date, cost, author, type
  while(fgets(buffer, BUFSIZ, fp) != NULL){
    date = (time_t) atol(strtok(buffer, ","));
    cost = atof(strtok(NULL, ","));
    author = strdup(strtok(NULL, ","));
    type = (ExpenseType) atoi(strtok(NULL, ","));

    account_add_exp(account, cost, date, type, author);
  }
}

void load_storage(Account * account){ 
  time_t current_time = time(NULL);
  struct tm * pTime = localtime(&current_time);
  strftime(displayed_month, 12, "< %m-%Y >", pTime);
  strftime(path, PATH_MAX_SIZE, PATH_STORAGE, pTime);

  FILE * fp;

  if((fp = fopen(path, "r")) == NULL){
    TraceLog(LOG_ERROR, "%s: unable to open filepath %s", LOG_PNAME, path);

    pTime->tm_mon = (pTime->tm_mon - 1) % 12;
    if(pTime->tm_mon == 11) // previous year
      pTime->tm_year--;

    strftime(path, PATH_MAX_SIZE, PATH_STORAGE, pTime);
    fp = fopen(path, "r");
    if(fp == NULL){
      TraceLog(LOG_ERROR, "%s: unable to open filepath %s", LOG_PNAME, path);
    }

    read_file(fp, account);

    float last_month = account_get_total(account);

    Expense exp;
    exp_init_now(&exp, last_month, INCOME, "last_month");
    save_expense(&exp);

    fclose(fp);

    // Try to open it again
    struct tm * pTimeCopy = localtime(&current_time);
    account->exp_nb = 0;
    strftime(path, PATH_MAX_SIZE, PATH_STORAGE, pTimeCopy);
    fp = fopen(path, "r");
  }

  read_file(fp, account);
  fclose(fp);
}

void filltab(Listview * v){
  for(int i = 0; i < account.exp_nb; i++){
    Expense exp = account.list[i];
    lv_add_row(v, exp.s_date, exp.author, exp.s_cost);
  }
}

// Call once when the lib is opened
void init()
{
  w = GetScreenWidth();
  h = GetScreenHeight();

  inputs[0] = input_def(10, "cost", 10, 10);
  inputs[1] = input_def(3, "type", 170, 10);
  inputs[2] = input_def(10, "author", 330, 10);

  TraceLog(LOG_INFO, "size of: %d", sizeof(headers) / sizeof(headers[0]));
  listview = lv_create(10, 60, w - 20, h - 130, 3, 30, headers);

  account = (Account) {
    .list = (Expense *) arena_alloc(&a, sizeof(Expense) * MAX_EXPENSES),
    .exp_nb = 0,
    .name = "Yohan",
    .max_exp_nb = MAX_EXPENSES
  };

  load_storage(&account);
  current_total = account_get_total(&account);
  TraceLog(LOG_INFO, "%s: account solde %6.2f", LOG_PNAME, current_total);
    
  inputs[selected].state = IS_SELECTED;
  filltab(&listview);
  mkdir(PATH_STORAGE, 0755);
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
  int curr_w = GetScreenWidth();
  int curr_h = GetScreenHeight();

  if(curr_w != w || curr_h != h){
    w = curr_w;
    h = curr_h;
    lv_set_size(&listview, w - 20, h - 130);
  }

  // Put a limit to add footer and + moving screen fucktin

  if(IsKeyPressed(KEY_TAB)){
    inputs[selected].state = 0;
    frm_counter = 0;
    selected++;
    if(selected >= INPUT_SIZE) selected = 0;
    inputs[selected].state = IS_SELECTED;
  }

  if(IsKeyPressed(KEY_DOWN) || (IsKeyDown(KEY_DOWN) && IsKeyDown(KEY_LEFT_CONTROL))){
    lv_row++;
    if(lv_row > account.exp_nb) lv_row = account.exp_nb - 1;
  }

  if(IsKeyPressed(KEY_UP) || (IsKeyDown(KEY_UP) && IsKeyDown(KEY_LEFT_CONTROL))){
    lv_row--;
    if(lv_row <= 0) lv_row = 0;
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
    if(isalnum(key) || key == '.'){
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
    ClearBackground(VAMPIREDARK);

    for(int i = 0; i < INPUT_SIZE; i++){
      input_draw(&inputs[i]);
    }

    DrawText(displayed_month, w - 155, 15, TEXT_SIZE, RAYWHITE);

    lv_draw(&listview, lv_row);

    DrawRectangle(w - 160, h - 40, 150, 30, DARKGRAY);
    DrawRectangleLines(w - 160, h - 40, 150, 30, BLACK);
    DrawText(TextFormat("%.2f", current_total), w - 154, h - 35, TEXT_SIZE, BLACK); 

    // TODO: DrawLine and values each month on 6 last months
  EndDrawing();
}

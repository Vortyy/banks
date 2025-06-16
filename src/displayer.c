#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <raylib.h>
#include <ctype.h>

#include "bank.h" // Expenses && Account
#include "input.h" // Custom Input

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
#define PATH_STORAGE "./store.csv"

#define MAX_EXPENSES 100
#define CSV_DELIMITER ","

#define MAX_LV_ROW 1000
#define LV_ORDER_REVERSER 0

#define VAMPIREDARK CLITERAL(Color){13, 2, 8, 255}

#define SET_CENTERED(box_size, elemnt_size) \
  ((box_size / 2) - (elemnt_size / 2))

void reset_inputs();
int store_inputs();
void save_expense(Expense * exp);
void load_storage(Account * account);

// Headers ???
typedef struct __listview_struct {
  int x;
  int y;

  int width;
  int height;

  int nb_col;
  int nb_row;
  int cell_w;
  int cell_h;
  
  char * tab[MAX_LV_ROW];
} Listview;

Listview lv_create(int x, int y, int width, int height, int nb_column, int cell_h)
{
  return (Listview){
    .x = x,
    .y = y,
    .width = width,
    .height = height,
    .nb_col = nb_column,
    .nb_row = 0,
    .cell_w = width/nb_column,
    .cell_h = cell_h
  };
}

/* it's a user burden to check that boundaries are respected */
/* arg are displayed from left to right */
void lv_add_row(Listview * self, ...)
{
  va_list ap;
  va_start(ap, self);
  int start_idx = self->nb_col * self->nb_row;

  for(int i = 0; i < self->nb_col; i++){
    char * value = va_arg(ap, char *);
    self->tab[start_idx + i] = value;
  }

  va_end(ap);
  self->nb_row++;
}

// TODO: order
void lv_draw(Listview * self, int starting_row)
{
  int i = starting_row >= self->nb_row ? self->nb_row - 1 : starting_row;

  BeginScissorMode(self->x, self->y, self->width, self->height);
  DrawRectangle(self->x, self->y, self->width, self->height, DARKGRAY);

  int printed_rows = 0;
  // Content list
  for(; (i < self->nb_row) && (printed_rows * self->cell_h < self->height); i++){
    for(int j = 0; j < self->nb_col; j++){
      Color bg_color = (i%2 == 0) ? LIGHTGRAY : GRAY; 
      DrawRectangle(self->x + (self->cell_w * j), self->y + (self->cell_h * printed_rows), self->cell_w, self->cell_h, bg_color);
      // Mesure text
      int idx = self->nb_col * i + j;
      int text_size = MeasureText(self->tab[idx], TEXT_SIZE);
      DrawText(self->tab[idx], self-> x + SET_CENTERED(self->cell_w, text_size) + (self->cell_w * j), self->y + (self->cell_h * printed_rows) + 5, TEXT_SIZE, BLACK);//Author
      //DrawRectangle(self->x + SET_CENTERED(self->cell_w, text_size) + (self->cell_w * j), self->y + (self->cell_h * printed_rows) + 5, text_size, 5 ,RAYWHITE);//Author
      //DrawRectangle(self->x + self->cell_w * j, self->y + (self->cell_h * printed_rows), text_size, 5 , BLUE);//Author
    }

    printed_rows++;
  }

  // Draw expenses list container
  DrawRectangleLines(self->x, self->y, self->width, self->height, BLACK);

  // Draw 3 lines + headers
  for(int i = 1; i < self->nb_col; i++){
    int line_x = self->cell_w * i + self->x;
    DrawLineEx((Vector2){line_x, self->y}, (Vector2){line_x, self->y + self->height}, 1.f, BLACK);
  }

  if(i != self->nb_row)
    if((frm_counter / 30) % 2 == 0)
      DrawTriangle(
        (Vector2) {self->x + self->width/2 - 10, self->y + self->height - 20}, 
        (Vector2) {self->x + self->width/2, self->y + self->height - 5},
        (Vector2) {self->x + self->width/2 + 10, self->y + self->height - 20}, 
        RAYWHITE);

  EndScissorMode();
}

/* Window && Globals variables */

/* Arena allocator declaration */
Arena a = {0};

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
  
  return 0;
}

void reset_inputs(){
  for(int i = 0; i < INPUT_SIZE; i++){
    input_reset(&inputs[i]);
  }
  selected=0;
}

void save_expense(Expense * exp){
  FILE * fp;

  if((fp = fopen(PATH_STORAGE, "a+")) == NULL){
    TraceLog(LOG_ERROR, "%s: unable to open filepath %s", LOG_PNAME, PATH_STORAGE);
  }

  // date, cost, author, type
  int ret = fprintf(fp, "%ld,%f,%s,%d\n", exp->date, exp->cost, exp->author, exp->type);

  fclose(fp);
} 

void load_storage(Account * account){ 
  float cost;
  int type;
  time_t date;
  char * author;

  FILE * fp;

  if((fp = fopen(PATH_STORAGE, "r")) == NULL){
    TraceLog(LOG_ERROR, "%s: unable to open filepath %s", LOG_PNAME, PATH_STORAGE);
    return;
  }

  char buffer[BUFSIZ];
  // date, cost, author, type
  while(fgets(buffer, BUFSIZ, fp) != NULL){
    date = (time_t) atol(strtok(buffer, ","));
    cost = atof(strtok(NULL, ","));
    author = strdup(strtok(NULL, ","));
    type = (ExpenseType) atoi(strtok(NULL, ","));

    account_add_exp(account, cost, date, type, author);
  }
   
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

  listview = lv_create(10, 60, w - 20, h - 130, 3, 30);

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

    lv_draw(&listview, lv_row);

    DrawRectangle(w - 160, h - 40, 150, 30, DARKGRAY);
    DrawRectangleLines(w - 160, h - 40, 150, 30, BLACK);
    DrawText(TextFormat("%6.2f", current_total), w - 155, h - 35, TEXT_SIZE, BLACK); 

    // TODO: DrawLine and values each month on 6 last months
    // TODO: total en bas
  EndDrawing();
}

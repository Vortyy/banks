#include <stdio.h>
#include <raylib.h>
#include <ctype.h>
#include <time.h>

// TODO: Maybe created a shared context to save data during runner exec
// Late binding can be fun -> https://stackoverflow.com/questions/32347910/oop-in-c-implicitly-pass-self-as-parameter

// Thx Tsoding and al.
#define ARENA_IMPLEMENTATION
#include "./includes/arena.h"

#define INPUT_SIZE 3
#define TEXT_SIZE 20
#define FRM_COUNTER_MAX_VALUE 2000000
#define LOG_PNAME "MY_DISPLAYER"

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

/* Account && Expenses */

typedef enum __expense_type {
  INCOME,
  OUTCOME
} ExpenseType;

typedef struct __expense_struct {
  int cost;
  ExpenseType type;
  time_t date;
  char * author;
  char * description;
} Expense;

void exp_init(Expense * exp, int cost, ExpenseType type, char * author, char * desc){
  exp->cost = cost;
  exp->type = type;
  exp->date = time(NULL);
  exp->author = author;
  exp->description = desc;
}

#define exp_print(exp) TraceLog(LOG_INFO, "%s: exp.cost: %d, exp.author: %s, exp.desc %s", LOG_PNAME, exp.cost, exp.author, exp.description)

/* Window */
int frm_counter = 0;
int w = 0;
int h = 0;
  
Expense expenses[4];

// Call once when the lib is opened
void init(){
  input_init(&inputs[0], 5, "Cost");
  input_init(&inputs[1], 3, "Type");
  input_init(&inputs[2], 10, "Author");

  exp_init(&expenses[0], 200, INCOME, "yohan", "description");
  exp_init(&expenses[1], 200, OUTCOME, "yohan", "description");
  exp_init(&expenses[2], 500, INCOME, "yohan", "description");
  exp_init(&expenses[3], 10, OUTCOME, "yohan", "description");

  for(int i = 0; i < 4; i++){
    exp_print(expenses[i]);
  }
}

// Call until the lib is closed/reloaded
void display(){
  // UPDATE
  w = GetScreenWidth();
  h = GetScreenHeight();

  if(IsKeyPressed(KEY_TAB)){
    frm_counter = 0;
    selected++;
    if(selected >= INPUT_SIZE) selected = 0;
  }

  if(IsKeyPressed(KEY_BACKSPACE)){
    input_del_char(&inputs[selected]);
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

    int box_size = ((w/2 - 20) / INPUT_SIZE);

    // Expenses list
    for(int i = 0; i < 4; i++){
      Expense * ptr_exp = expenses + i;
      int y = 65 + 30 * i; // starting size need to depend on text size

      Color bg_color = (ptr_exp->type == INCOME) ? GREEN : RED;
      DrawRectangle(10, 60 + 30 * i, w/2 - 21,  30, bg_color);

      // Text measurement
      int author_size = MeasureText(ptr_exp->author, TEXT_SIZE);
      int description_size = MeasureText(ptr_exp->description, TEXT_SIZE);
      int cost_size = MeasureText(TextFormat("%d", ptr_exp->cost), TEXT_SIZE);
      //int date_size = MeasureText(ptr_exp->date, TEXT_SIZE);

      // Write text
      // TODO: check if box_size > measure text and write a substring ???
      //DrawText(ptr_exp->date, set_centered(box_size, date_size) + 10, y, TEXT_SIZE, BLACK);//Date
      DrawText(ptr_exp->author, SET_CENTERED(box_size, author_size) + 10, y, TEXT_SIZE, BLACK);//Author
      DrawText(ptr_exp->description, SET_CENTERED(box_size, description_size) + (box_size * 1 + 10), y, TEXT_SIZE, BLACK);//Description
      DrawText(TextFormat("%d", ptr_exp->cost), SET_CENTERED(box_size, cost_size) + (box_size * 2 + 10), y, TEXT_SIZE, BLACK);//Cost
    }

    // Draw expenses list container
    DrawRectangleLines(10, 60, w/2 - 20, h - 100 , BLACK);

    // Draw 3 lines + headers
    for(int i = 1; i < INPUT_SIZE; i++){
      int x = box_size * i + 10;
      DrawLine(x, 60, x, h - 40, BLACK);
    }

  EndDrawing();
}

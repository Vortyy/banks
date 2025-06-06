#include <stdio.h>
#include <raylib.h>
#include <ctype.h>

// TODO: Maybe created a shared context to save data during runner exec

// Thx Tsoding and al.
#define ARENA_IMPLEMENTATION
#include "./includes/arena.h"

#define INPUT_SIZE 3
#define TEXT_SIZE 20

/* Input */ 
typedef struct {
  char * content;
  const char * placeholder;
  int current_pos;
  int char_limit; 
} Input;

Arena a = {0};
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

void input_add_char(Input * input, char c){
  if(input->current_pos < input->char_limit){
    input->content[input->current_pos] = c;
    input->content[input->current_pos + 1] = '\0';
    input->current_pos++;
  }
}

void input_del_char(Input * input){
  input->current_pos--;
  if(input->current_pos < 0) input->current_pos = 0;
  input->content[input->current_pos] = '\0';
}

// Call once at the window opening
void init(){
  input_init(&inputs[0], 5, "Cost");
  input_init(&inputs[1], 3, "Type");
  input_init(&inputs[2], 10, "Author");
}

// Call until the windows is close
void display(){
  // UPDATE
  if(IsKeyPressed(KEY_TAB)){
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
      } else {
        DrawRectangleLines(x, rect_prop[0], rect_prop[1], rect_prop[2], BLACK);
      }
    }
  EndDrawing();
}

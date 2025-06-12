#ifndef INPUT_H
#define INPUT_H

#include <raylib.h>
#include "arena.h"

extern Arena a; // WARNING: Declared inside displayer.c
extern int frm_counter;

#define INPUT_TEXT_SIZE 20
#define INPUT_DEFAULT_WIDTH 150
#define INPUT_DEFAULT_HEIGHT 30

#define IS_SELECTED 1

/* Input (24bytes) */ 
typedef struct __input_struct {
  char * content;
  const char * placeholder;
  int char_limit; 
  int current_pos;

  int x;
  int y;
  int width;
  int height;

  int state;
} Input;

#define input_def(char_limit, placeholder, x, y) input_create(char_limit, placeholder, x, y, INPUT_DEFAULT_WIDTH, INPUT_DEFAULT_HEIGHT)

Input input_create(int char_limit, const char * placeholder, int x, int y, int width, int height);

void input_init(Input * input, int char_limit, const char * placeholder);
void input_add_char(Input * self, char c);
void input_del_char(Input * self);
void input_reset(Input * self);

void input_draw(Input * self); // This method is implemented by the renderer

#endif

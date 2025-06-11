#ifndef INPUT_H
#define INPUT_H

#include "arena.h"

extern Arena a; // WARNING: Declared inside displayer.c

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

void input_draw(Input * self, int x, int y, int width, int heigth, int selected); // This method is implemented by the renderer

#endif

#include "input.h"

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

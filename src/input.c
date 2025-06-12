#include "input.h"

Input input_create(int char_limit, const char * placeholder, int x, int y, int width, int height)
{
  char * ptr_content = (char *) arena_alloc(&a, sizeof(char) * (char_limit + 1));
  *ptr_content = '\0';

  return (Input) {
      .content = ptr_content,
      .placeholder = placeholder,
      .char_limit = char_limit,
      .current_pos = 0,
      .x = x,
      .y = y,
      .width = width,
      .height = height,
      .state = 0
  };
}

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

void input_draw(Input * self)
{
  // Background
  DrawRectangle(self->x, self->y, self->width, self->height, DARKGRAY);

  if(self->current_pos == 0)
    DrawText(self->placeholder, self->x + 5, self->y + 5, INPUT_TEXT_SIZE, GRAY);

  DrawText(self->content, self->x + 5, self->y + 5, INPUT_TEXT_SIZE, RED);
  DrawRectangleLines(self->x, self->y, self->width, self->height, BLACK);

  if(self->state == IS_SELECTED){
    DrawRectangleLines(self->x, self->y, self->width, self->height, RED);
    if(self->current_pos < self->char_limit && self->current_pos > 0){
      if((frm_counter/20) % 2 == 0) DrawText("_", self->x + 8 + MeasureText(self->content, INPUT_TEXT_SIZE), self->y + 5, INPUT_TEXT_SIZE, RED);
    }
  }
}

#include "listview.h"

Listview lv_create(int x, int y, int width, int height, int nb_column, int cell_h, char ** headers)
{
  return (Listview){
    .x = x,
    .y = y,
    .width = width,
    .height = height,
    .nb_col = nb_column,
    .nb_row = 0,
    .cell_w = width/nb_column,
    .cell_h = cell_h,
    .headers = headers
  };
}

void lv_set_size(Listview * self, int width, int height){
  self->width = width;
  self->height = height;

  self->cell_w = width / (self->nb_col);
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

void lv_clear(Listview * self){
  self->nb_row = 0;
}

// TODO: order
void lv_draw(Listview * self, int starting_row)
{
  int i = (starting_row >= self->nb_row && self->nb_row != 0) ? self->nb_row - 1 : starting_row;
  int printed_rows = 0;

  // Draw headers
  if(self->headers != NULL){
    for(int h = 0; h < self->nb_col; h++){
      int text_size = MeasureText(self->headers[h], TEXT_SIZE);
      DrawText(self->headers[h], self-> x + SET_CENTERED(self->cell_w, text_size) + (self->cell_w * h), self->y + 5, TEXT_SIZE, RAYWHITE);//Author
    }
    printed_rows++;
  }

  BeginScissorMode(self->x, self->y + (self->cell_h * printed_rows), self->width, self->height);
  DrawRectangle(self->x, self->y, self->width, self->height, DARKGRAY);

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


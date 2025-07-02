#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <stdlib.h>
#include <raylib.h>
extern int frm_counter;

#define MAX_LV_ROW 1000
#define LV_ORDER_REVERSER 0

#define TEXT_SIZE 20
#define SET_CENTERED(box_size, elemnt_size) \
  ((box_size / 2) - (elemnt_size / 2))

typedef struct __listview_struct {
  int x;
  int y;

  int width;
  int height;

  int nb_col;
  int nb_row;
  int cell_w;
  int cell_h;
  
  char ** headers;
  char * tab[MAX_LV_ROW];
} Listview;

Listview lv_create(int x, int y, int width, int height, int nb_column, int cell_h, char ** headers);
#define lv_noh_create(x, y, width, height, nb_col, cell_h) lv_create(x, y, width, height, nb_col, cell_h, NULL)

void lv_set_size(Listview * self, int width, int height);
void lv_add_row(Listview * self, ...);
void lv_clear(Listview * self);
void lv_draw(Listview * self, int starting_row);

#endif

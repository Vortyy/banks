#include <stdio.h>
#include <raylib.h>

// TODO: use arena_allocator to do some stuff

void display(){
  BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Congrat Hot reloading properly", 190, 200, 20, LIGHTGRAY);
    DrawText("By Yohan CHABOT", 190, 220, 20, RED);
  EndDrawing();
}

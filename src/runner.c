#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#include <raylib.h>

#define PNAME "MY_RUNNER"
#define DISPLAYER_PATH "./displayer.so"

/* function provided by the displayer */
void (*init_displayer_fct)(); 
void (*displayer_fct)();
void (*clear_displayer_fct)();
void * displayer_module;

struct stat sb; /* stat about displayer.so file */

/* loadLib : loads a shared library that contains a displayer in raylib */
void loadLib(){
  if(displayer_module != NULL) {
    clear_displayer_fct();
    dlclose(displayer_module);
  }

  TraceLog(LOG_INFO, "%s: Loading shared lib...", PNAME);

  // TODO: close when program spend more than 5sec to dlopen
  // TODO: find a best way to open lib than brute forcing dlopen
  while((displayer_module = dlopen(DISPLAYER_PATH, RTLD_NOW)) == NULL);
  
  init_displayer_fct = dlsym(displayer_module, "init");
  displayer_fct = dlsym(displayer_module, "display");
  clear_displayer_fct = dlsym(displayer_module, "clear");

  TraceLog(LOG_INFO, "%s: Shared lib loaded", PNAME);
}

int setStat(){
  int result = stat(DISPLAYER_PATH, &sb);
  if(result == -1){
    TraceLog(LOG_ERROR, "%s: filepath doesn't exist %s", PNAME, DISPLAYER_PATH);
  }
  return result;
}

int main(int argc, char** argv) {
  time_t last_mod = 0;

  const int w = 800;
  const int h = 450;
    
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(w, h, "raylib go back to C");
  SetTargetFPS(60);

  while(!WindowShouldClose() && !setStat())
  {
    // Checks if an the lib has been updated since the last loading
    if(difftime(sb.st_mtime, last_mod) > 0){
      last_mod = sb.st_mtime;
      loadLib();
      init_displayer_fct();
    }

    displayer_fct();
 }

  if(displayer_module != NULL)
    dlclose(displayer_module);

  return 0;
}

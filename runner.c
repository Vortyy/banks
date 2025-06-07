#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#include <raylib.h>

#define PNAME_BUFSIZ 20

const char libpath[] = "./displayer.so";
char pname[PNAME_BUFSIZ] = "NO_NAME";

void (*init_displayer_fct)();
void (*displayer_fct)();
void * displayer_module;

struct stat sb;

/* loadLib : loads a shared library that contains a displayer in raylib */
void loadLib(){
  if(displayer_module != NULL) {
    dlclose(displayer_module);
  }

  TraceLog(LOG_INFO, "%s: Loading shared lib...", pname);

  // TODO: close when program spend more than 5sec to dlopen
  // TODO: find a best way to open lib than brute forcing dlopen
  while((displayer_module = dlopen(libpath, RTLD_NOW)) == NULL);
  
  init_displayer_fct = dlsym(displayer_module, "init");
  displayer_fct = dlsym(displayer_module, "display");

  TraceLog(LOG_INFO, "%s: Shared lib loaded", pname);
}

int setStat(){
  int result = stat(libpath, &sb);
  if(result == -1){
    TraceLog(LOG_ERROR, "%s: filepath doesn't exist %s", pname, libpath);
  }
  return result;
}

void setPname(char *value){
  char * p;

  // skip "./"
  value++;
  value++;

  p = pname;
  while(*value){
    *p++ = toupper(*value++);
  }

  *p = '\0';
}

int main(int argc, char** argv) {
  setPname(*argv);
  
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

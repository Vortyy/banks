CC = clang
CFLAGS = all
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
SRC_FILES = src/bank.c src/displayer.c src/input.c src/listview.c

# Generates shared libs for hot reloading
all: displayer runner
	./rayrunner

displayer:
	$(CC) $(SRC_FILES) -iquote ./includes -shared -fPIC $(LIBS) -o displayer.so  

# Runner that run the shared lib inside a Raylib window
runner:
	$(CC) src/runner.c -DDEBUG $(LIBS) -o rayrunner 

clean:
	rm -rf rayrunner displayer.so 

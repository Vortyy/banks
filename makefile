CC = clang
CFLAGS = all
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Generates shared libs for hot reloading
all:
	$(CC) displayer.c -shared -fPIC $(LIBS) -o displayer.so  

# Runner that run the shared lib inside a Raylib window
runner:
	$(CC) runner.c -DDEBUG $(LIBS) -o rayrunner 

rlaunch: runner
	./rayrunner

clean:
	rm -rf rayrunner displayer.so 

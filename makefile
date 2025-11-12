CC = clang
CFLAGS = all
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
SRC_FILES = src/bank.c src/displayer.c src/input.c src/listview.c

# Generates shared libs for hot reloading
all:
	$(CC) $(SRC_FILES) -iquote ./includes -shared -fPIC $(LIBS) -o displayer.so  

# Runner that run the shared lib inside a Raylib window
runner:
	$(CC) src/runner.c -DDEBUG $(LIBS) -o rayrunner 

install: bank.o
	$(CC) $^ src/term.c -iquote ./includes -o bankt
	mv bankt /usr/bin/bankt

term: bank.o
	$(CC) $^ src/term.c -DDEBUG -iquote ./includes -o bankt

bank.o:
	$(CC) -c src/bank.c -iquote ./includes -o bank.o

rlaunch: runner
	./rayrunner

clean:
	rm -rf *.o bankt rayrunner displayer.so 

SRC=battery
OBJ=$(SRC)
all:
	gcc -Wall -o $(OBJ) $(OBJ).c `pkg-config --libs --cflags libnotify` 
clean:
	rm $(OBJ)

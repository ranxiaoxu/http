ROOT_PATH=$(shell pwd)
BIN=httpd
SRC=httpd.c
OBJ=$(SRC:.c=.o)
CC=gcc
FLAGS=-D _DEBUG_
LDFLAGS=-lpthread

$(BIN):$(SRC)
	$(CC) -o $@ $^ $(LDFLAGS) 
%.o:%.c
	$(CC) -c $< $(FLAGS)

.PHONY:debug
debug:
	@echo $(ROOT_PATH)
	@echo $(BIN)
	@echo $(SRC)
	@echo $(OBJ)

.PHONY:clean
clean:
	rm -f *.o $(BIN)

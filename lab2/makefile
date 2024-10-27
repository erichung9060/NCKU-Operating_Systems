TARGET 	= my_shell
CC     	= gcc
FLAGS  	= -Wall
OBJ    	= builtin.o command.o shell.o
INCLUDE = ./include/
SRC		= ./src/

all: $(TARGET) 

$(TARGET): my_shell.c $(OBJ) 
	$(CC) $(FLAGS) -o $(TARGET) $(OBJ) $<

%.o: ${SRC}%.c ${INCLUDE}%.h
	$(CC) $(FLAGS) -c $<

.PHONY: clean
clean:
	rm -f ${TARGET} *.o out*
clean_obj:
	rm -f *.o

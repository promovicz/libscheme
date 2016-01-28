INC=-I..
LIB=-L..
CC=cc
CFLAGS=-O $(INC)

OBJS = posix_file.o posix_proc.o

posixscheme: $(OBJS) main.o
	$(CC) $(CFLAGS) -o posixscheme $(OBJS) main.o $(LIB) -lscheme -lm

clean:
	rm -f $(OBJS) main.o *~ posixscheme


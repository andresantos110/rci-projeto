OBJS	= udp.o teste.o
SOURCE	= udp.c teste.c
HEADER	= udp.h
OUT	= teste
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

udp.o: udp.c
	$(CC) $(FLAGS) udp.c 

teste.o: teste.c
	$(CC) $(FLAGS) teste.c 


clean:
	rm -f $(OBJS) $(OUT)

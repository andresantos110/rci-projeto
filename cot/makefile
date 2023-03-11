OBJS	= udp.o cot.o
SOURCE	= udp.c cot.c
HEADER	= udp.h cot.h
OUT	= cot
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

udp.o: udp.c
	$(CC) $(FLAGS) udp.c 

cot.o: cot.c
	$(CC) $(FLAGS) cot.c 


clean:
	rm -f $(OBJS) $(OUT)

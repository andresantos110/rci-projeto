OBJS	= udp.o cot.o select.o
SOURCE	= udp.c cot.c select.c
HEADER	= udp.h cot.h select.h
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
	
select.o: select.c
	$(CC) $(FLAGS) select.c 


clean:
	rm -f $(OBJS) $(OUT)

OBJS	= udp.o cot.o select.o aux.o commTCP.o
SOURCE	= udp.c cot.c select.c aux.c commTCP.c
HEADER	= udp.h cot.h select.h aux.h commTCP.h
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
	
aux.o: aux.c
	$(CC) $(FLAGS) aux.c 
	
commTCP.o: commTCP.c
	$(CC) $(FLAGS) commTCP.c


clean:
	rm -f $(OBJS) $(OUT)

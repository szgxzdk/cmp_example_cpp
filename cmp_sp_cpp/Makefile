OBJS = shortest_path.o graph.o func.o barrier.o
CC = alpha-unknown-linux-gnu-g++ 

sp: $(OBJS)
	$(CC)  -O3 -g3 -w -static  $(OBJS) -o sp

func.o: func.s
	$(CC) -O3 -g3 -w -static  -c func.s
shortest_path.o: shortest_path.cc
	$(CC) -O3 -g3 -w -static  -c shortest_path.cc
graph.o: graph.cc
	$(CC) -O3 -g3 -w -static  -c graph.cc
barrier.o: barrier.c
	$(CC) -O3 -g3 -w -static  -c barrier.c

clean:
	rm -f *.o sp


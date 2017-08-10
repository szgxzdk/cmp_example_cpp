OBJS = hello.o  func.o barrier.o
CC = alpha-unknown-linux-gnu-g++ 

hello: $(OBJS)
	$(CC)  -O3 -g3 -w -static  $(OBJS) -o hello -lm

func.o: func.s
	$(CC) -O3 -g3 -w -static  -c func.s
hello.o: hello.c
	$(CC) -O3 -g3 -w -static  -c hello.c
barrier.o: barrier.c
	$(CC) -O3 -g3 -w -static  -c barrier.c

clean:
	rm -f *.o hello 


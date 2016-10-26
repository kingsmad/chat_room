OBJS = main.o terminal.o server.o client.o
CC = clang++ 
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG) -std=c++11
LFLAGS = -Wall $(DEBUG)

MIPSsim : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o chat 

server.o: server.h 
	$(CC) $(CFLAGS) server.cpp
client.o: client.h
	$(CC) $(CFLAGS) client.cpp
terminal.o: terminal.h server.h server.cpp client.h client.cpp
	$(CC) $(CFLAGS) terminal.o
main.o: terminal.o server.o client.o
	$(CC) $(CFLAGS) terminal.o

clean:
	    \rm *.o chat 

tar:
	    tar cfv chat.tar *.h *.cpp *.o chat 

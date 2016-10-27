OBJS = terminal.o server.o client.o main.o
CC = clang++ 
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG) -std=c++11
LFLAGS = -Wall $(DEBUG)

chat : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o chat 

client.o: client.h
	$(CC) $(CFLAGS) client.cpp
server.o: server.h
	$(CC) $(CFLAGS) server.cpp
terminal.o: terminal.h server.o client.o
	$(CC) $(CFLAGS) terminal.cpp
main.o: terminal.o
	$(CC) $(CFLAGS) main.cpp 

clean:
	    \rm *.o chat 
tar:
	    tar cfv chat.tar *.h *.cpp *.o chat 

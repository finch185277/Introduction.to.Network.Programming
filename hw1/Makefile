all: server client

chat-room.o: chat-room.c
	gcc -c -o chat-room.o $^

server: server.c chat-room.o
	gcc -o server $^

client: client.c
	gcc -o client $^

test: all
	gcc -o test test.c
	stdbuf -o0 ./test

clean:
	rm -f server client chat-room.o test

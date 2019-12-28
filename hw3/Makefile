test: test.c
	gcc test.c -o test

server: server.c
	gcc server.c -o server

client: client.c
	gcc client.c -o client

all: test server client

.PHONY: demo clean

demo: all
	./test . 8888 1

clean:
	rm test server client

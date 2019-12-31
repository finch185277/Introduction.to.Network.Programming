TEST_OUTPUT = tom frank

all: test-script server client

test-script: test.c
	gcc test.c -o test

server: server.cpp
	g++ server.cpp -o server

client: client.cpp
	g++ client.cpp -o client

.PHONY: demo clean

test: all
	./test . 8888 1

clean:
	rm -rf test server client $(TEST_OUTPUT)

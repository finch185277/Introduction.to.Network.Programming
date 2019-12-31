TEST_OUTPUT = 1 2 3 4 tom frank
EXEC_FILES = test server client

all: test-script server client

test-script:
	gcc test.c -o test

server:
	g++ server.cpp -o server

client:
	g++ client.cpp -o client

.PHONY: demo clean

test: all
	./test . 8686 1

clean:
	rm -rf $(EXEC_FILES) $(TEST_OUTPUT)

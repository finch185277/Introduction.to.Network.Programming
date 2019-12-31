TEST_OUTPUT = tom frank
EXEC_FILES = test server client
TEST_SCRIPT = test

all: script server client

script:
	gcc test.c -o $(TEST_SCRIPT)

server:
	g++ server.cpp -o server

client:
	g++ client.cpp -o client

.PHONY: test clean

test: all
	./$(TEST_SCRIPT) . 8686 1

clean:
	rm -rf $(EXEC_FILES) $(TEST_OUTPUT)

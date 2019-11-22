CXX=gcc

all: server client

server: server.c
	$(CXX) -o server $^

client: client.c
	$(CXX) -o client $^

.PHONY: clean

clean:
	rm -rf server client

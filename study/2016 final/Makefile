EXEC_FILES = server client pingsubnet myifconfig nslookup ifconfig

p1: p1s.cpp p1c.cpp
	g++ -o server p1s.cpp
	g++ -o client p1c.cpp

p2: p2.c
	gcc -o pingsubnet p2.c

p3: server.cpp p3.cpp
	g++ -o server server.cpp
	g++ -o client p3.cpp

p4: p4.cpp
	g++ -o myifconfig p4.cpp

nslookup: nslookup.c
	gcc -o nslookup nslookup.c

ifconfig: ifconfig.c
	gcc -o ifconfig ifconfig.c

.PHONY: clean

clean:
	rm -rf $(EXEC_FILES)

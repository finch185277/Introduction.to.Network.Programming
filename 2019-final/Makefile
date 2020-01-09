EXEC_FILES = server client nslookup myifconfig

p1: p1.c
	gcc -o nslookup p1.c

p2: p2s.c p2c.c
	gcc -o server p2s.c
	gcc -o client p2c.c

p3: p3.c
	gcc -o myifconfig p3.c

.PHONY: clean

clean:
	rm -rf $(EXEC_FILES)

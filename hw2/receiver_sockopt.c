// server
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define BUF_SIZE 1000

struct frame {
  long int ID;
  long int length;
  char data[BUF_SIZE];
};

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s [save filename] [bind port]\n", argv[0]);
    exit(0);
  }

  return 0;
}

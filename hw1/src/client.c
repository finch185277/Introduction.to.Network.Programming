#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s <SERVER IP> <SERVER PORT>\n", argv[0]);
    exit(0);
  }
  return 0;
}

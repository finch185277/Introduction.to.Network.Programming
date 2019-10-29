#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define MAXLINE 1000

int main(int argc, char **argv) {
  int sockfd, n;
  char buff[MAXLINE];
  struct sockaddr_in server_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(13);
  inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

  connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

  while ((n = read(sockfd, buff, MAXLINE)) > 0) {
    buff[n] = 0;
    if (fputs(buff, stdout) == EOF) {
      printf("fall to print received data\n");
      exit(1);
    }
  }
  exit(0);
}

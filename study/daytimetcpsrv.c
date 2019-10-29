#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define MAXLINE 1000
#define LISTENQ 16

int main(int argc, char **argv) {
  int listenfd, connfd;
  socklen_t len;
  struct sockaddr_in server_addr, client_addr;
  char buff[MAXLINE];
  time_t ticks;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(13);

  bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

  listen(listenfd, LISTENQ);

  for (;;) {
    len = sizeof(client_addr);
    connfd = accept(listenfd, (struct sockaddr *)&client_addr, &len);
    inet_ntop(AF_INET, &client_addr.sin_addr, buff, sizeof(buff));
    printf("connection from %s, port %d\n", buff, ntohs(client_addr.sin_port));

    ticks = time(NULL);
    snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
    write(connfd, buff, strlen(buff));

    close(connfd);
  }
  return 0;
}

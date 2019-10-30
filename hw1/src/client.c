#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define LINE_MAX 1024

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s <SERVER IP> <SERVER PORT>\n", argv[0]);
    exit(0);
  }

  int sock_fd;
  char buf[LINE_MAX];
  struct sockaddr_in srv_addr;

  memset(&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_addr.s_addr = inet_addr(argv[1]);
  srv_addr.sin_port = htons(atoi(argv[2]));

  sock_fd = socket(AF_INET, SOCK_STREAM, 0);

  connect(sock_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));

  int max_fd = (sock_fd > STDIN_FILENO ? sock_fd : STDIN_FILENO) + 1;
  fd_set rset;
  FD_ZERO(&rset);

  for (;;) {
    FD_SET(STDIN_FILENO, &rset);
    FD_SET(sock_fd, &rset);
    select(max_fd, &rset, NULL, NULL, NULL);

    if (FD_ISSET(STDIN_FILENO, &rset)) {
      int n = read(STDIN_FILENO, buf, LINE_MAX - 1);
      buf[n] = '\0';
      if (strcmp(buf, "exit\n") == 0) {
        close(sock_fd);
        break;
      }
      write(sock_fd, buf, n);
    }

    if (FD_ISSET(sock_fd, &rset)) {
      int n = read(sock_fd, buf, LINE_MAX - 1);
      if (n == 0) {
        printf("Connection closed by server\n");
        close(sock_fd);
        break;
      }
      buf[n] = '\0';
      fprintf(stdout, "%s", buf);
    }
  }

  return 0;
}

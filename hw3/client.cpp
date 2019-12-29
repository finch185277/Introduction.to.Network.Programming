#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define LINE_MAX 1024

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s <SERVER IP> <SERVER PORT> <USERNAME>\n", argv[0]);
    exit(0);
  }

  printf("Welcome to the dropbox-like server: %s\n", argv[3]);

  struct hostent *host = gethostbyname(argv[1]);

  struct sockaddr_in srv_addr;
  memset(&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_addr.s_addr = inet_addr(argv[1]);
  memcpy(&srv_addr.sin_addr, host->h_addr, host->h_length);
  srv_addr.sin_port = htons(atoi(argv[2]));

  int sock_fd;
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);

  connect(sock_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
  write(sock_fd, argv[3], sizeof(argv[3]));

  fd_set r_set;
  FD_ZERO(&r_set);
  int max_fd = (sock_fd > STDIN_FILENO ? sock_fd : STDIN_FILENO) + 1;

  for (;;) {
    FD_SET(STDIN_FILENO, &r_set);
    FD_SET(sock_fd, &r_set);
    select(max_fd, &r_set, NULL, NULL, NULL);

    if (FD_ISSET(STDIN_FILENO, &r_set)) {
      char buf[LINE_MAX];
      int n = read(STDIN_FILENO, buf, LINE_MAX - 1);
      buf[n] = '\0';
      if (strcmp(buf, "exit\n") == 0) {
        close(sock_fd);
        break;
      }
      write(sock_fd, buf, n);
    }

    if (FD_ISSET(sock_fd, &r_set)) {
      char buf[LINE_MAX];
      int n = read(sock_fd, buf, LINE_MAX);
      if (n == 0) {
        close(sock_fd);
        break;
      }
      fprintf(stdout, "%s", buf);
    }
  }

  return 0;
}

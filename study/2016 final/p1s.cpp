#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <server port>\n", argv[0]);
    exit(0);
  }

  struct sockaddr_in srv_addr;
  bzero(&srv_addr, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(atoi(argv[1]));
  srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

  struct timeval tv;
  tv.tv_sec = 3;
  tv.tv_usec = 0;
  setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  bind(sock_fd, (const struct sockaddr *)&srv_addr, sizeof(srv_addr));

  for (;;) {
    int nread;
    char buf[1024];
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    nread =
        recvfrom(sock_fd, buf, 1024, 0, (struct sockaddr *)&cli_addr, &cli_len);
    if (nread < 0) {
      if (errno == EWOULDBLOCK)
        continue;
      else
        perror("recvfrom() error");
    }

    int a = atoi(buf);

    nread =
        recvfrom(sock_fd, buf, 1024, 0, (struct sockaddr *)&cli_addr, &cli_len);
    if (nread < 0) {
      if (errno == EWOULDBLOCK)
        continue;
      else
        perror("recvfrom() error");
    }

    int b = atoi(buf);
    int c = a + b;
    sprintf(buf, "%d", c);

    sendto(sock_fd, buf, strlen(buf), 0, (struct sockaddr *)&cli_addr, cli_len);
  }
}

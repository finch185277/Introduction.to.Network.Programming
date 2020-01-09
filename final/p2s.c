#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LINE_MAX 1024

int wait_flag = 1;

void handler(int signo) {
  wait_flag = 0;
  return;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <server port>\n", argv[0]);
    exit(0);
  }

  volatile unsigned int i = 0;

  struct sockaddr_in srv_addr;
  bzero(&srv_addr, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(atoi(argv[1]));
  srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

  bind(sock_fd, (const struct sockaddr *)&srv_addr, sizeof(srv_addr));

  fcntl(sock_fd, F_SETOWN, getpid());
  fcntl(sock_fd, F_SETFL, FASYNC);

  struct sigaction act;
  act.sa_handler = handler;
  act.sa_flags = 0;
  act.sa_restorer = NULL;
  sigaction(SIGIO, &act, NULL);

  for (;;) {
    i++;
    if (wait_flag == 0) {
      char buf[LINE_MAX];
      struct sockaddr_in cli_addr;
      socklen_t cli_len = sizeof(cli_addr);

      int nread = recvfrom(sock_fd, buf, LINE_MAX - 1, 0,
                           (struct sockaddr *)&cli_addr, &cli_len);

      if (nread == 0)
        break;
      if (nread < 0)
        continue;

      buf[nread] = '\0';

      if (strcmp(buf, "GetProgress\n") == 0) {
        sprintf(buf, "Progress: i = %u\n", i);
        sendto(sock_fd, buf, strlen(buf), 0, (struct sockaddr *)&cli_addr,
               cli_len);
      }
      wait_flag = 1;
    }
  }
  return 0;
}

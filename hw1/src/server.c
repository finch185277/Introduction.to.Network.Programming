#include "chat-room.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <SERVER PORT>\n", argv[0]);
    exit(0);
  }

  int listen_fd;
  int idx_bound = 0;
  struct Client *clients = malloc(CLIENT_MAX * sizeof(struct Client));
  for (int i = 0; i < CLIENT_MAX; i++)
    clients[i].fd = -1;

  struct sockaddr_in listen_addr;
  memset(&listen_addr, 0, sizeof(listen_addr));
  listen_addr.sin_family = AF_INET;
  listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  listen_addr.sin_port = htons(atoi(argv[1]));

  listen_fd = socket(AF_INET, SOCK_STREAM, 0);

  bind(listen_fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr));

  listen(listen_fd, 1024);

  fd_set all_set, r_set;
  FD_SET(listen_fd, &all_set);
  int max_fd = listen_fd + 1;

  for (;;) {
    r_set = all_set;
    int nready = select(max_fd, &r_set, NULL, NULL, NULL);

    if (FD_ISSET(listen_fd, &r_set)) {
      struct sockaddr_in cli_addr;
      int cli_len = sizeof(cli_addr);
      int cli_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_len);

      msg_broadcast(clients, idx_bound, "Someone is coming!");

      int idx = 0;
      for (; idx < CLIENT_MAX; idx++) {
        if (clients[idx].fd < 0) {
          clients[idx].fd = cli_fd;
          break;
        }
      }

      if (idx == CLIENT_MAX) {
        printf("Chat Room is full!\n");
        exit(0);
      }

      inet_ntop(AF_INET, &cli_addr.sin_addr, clients[idx].ip,
                sizeof(clients[idx].ip));
      clients[idx].port = ntohs(cli_addr.sin_port);

      user_come(clients, idx);

      if (idx > idx_bound) {
        idx_bound = idx;
      }
      if (cli_fd >= max_fd) {
        max_fd = cli_fd + 1;
      }
      FD_SET(cli_fd, &all_set);
      if (--nready)
        continue;
    }

    for (int idx = 0; idx <= idx_bound && nready >= 0; idx++) {
      if (FD_ISSET(clients[idx].fd, &r_set)) {
        --nready;

        char buf[LINE_MAX];
        int n = read(clients[idx].fd, buf, LINE_MAX - 1);
        if (n != 0) {
          buf[n] = '\0';
          def_cmd(clients, idx, idx_bound, buf);
        } else {
          close(clients[idx].fd);
          FD_CLR(clients[idx].fd, &all_set);
          clients[idx].fd = -1;
          user_leave(clients, idx, idx_bound);
        }
      }
    }
  }

  return 0;
}

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define CLIENT_MAX 10
#define LINE_MAX 1024

typedef struct Client {
  int fd;
  char name[16];
  char ip[16];
  int port;
} Client;

void msg_send(int fd, char *msg) {
  char buf[LINE_MAX];
  sprintf(buf, "[Server] %s\n", msg);
  write(fd, buf, strlen(buf));
}

void msg_broadcast(struct Client *clients, int idx_bound, char *msg) {
  for (int i = 0; i <= idx_bound; i++)
    if (clients[i].fd >= 0)
      msg_send(clients[i].fd, msg);
}

void user_come(struct Client *clients, int idx) {
  char buf[LINE_MAX];

  strcpy(clients[idx].name, "anonymous");

  sprintf(buf, "Hello, anonymous! From: %s:%d", clients[idx].ip,
          clients[idx].port);
  msg_send(clients[idx].fd, buf);
}

void user_leave(struct Client *clients, int idx, int idx_bound) {
  char buf[LINE_MAX];
  sprintf(buf, "%s is offline.", clients[idx].name);
  msg_broadcast(clients, idx_bound, buf);
}

void cmd_name(struct Client *clients, int idx, int idx_bound, char *name) {
  if (strcmp(name, "anonymous") == 0) {
    msg_send(clients[idx].fd, "ERROR: Username cannot be anonymous.");
    return;
  }
  for (int i = 0; i <= idx_bound; i++) {
    if (clients[i].fd >= 0 && i != idx &&
        strcmp(name, clients[idx].name) == 0) {
      char buf[LINE_MAX];
      sprintf(buf, "ERROR: %s has been used by others.", name);
      msg_send(clients[idx].fd, buf);
      return;
    }
  }
  int name_len = strlen(name);
  if (name_len < 2 || name_len > 12 ||
      strspn(name, "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz") !=
          name_len) {
    msg_send(clients[idx].fd,
             "ERROR: Username can only consists of 2~12 English letters.");
    return;
  }

  char buf[LINE_MAX];

  sprintf(buf, "You're now known as %s.", name);
  msg_send(clients[idx].fd, buf);

  int fd = clients[idx].fd;
  clients[idx].fd = -1;
  sprintf(buf, "%s is now known as %s.", clients[idx].name, name);
  msg_broadcast(clients, idx_bound, buf);
  clients[idx].fd = fd;

  strcpy(clients[idx].name, name);
}

void cmd_tell(struct Client *clients, int idx, int idx_bound, char *name,
              char *msg) {
  if (strcmp(clients[idx].name, "anonymous") == 0) {
    msg_send(clients[idx].fd, "ERROR: You are anonymous.");
    return;
  }
  if (strcmp(name, "anonymous") == 0) {
    msg_send(clients[idx].fd,
             "ERROR: The client to which you sent is anonymous.");
    return;
  }
  for (int i = 0; i <= idx_bound; i++) {
    if (clients[i].fd >= 0 && strcmp(clients[i].name, name) == 0) {
      char buf[LINE_MAX];

      msg_send(clients[idx].fd, "SUCCESS: Your message has been sent.");

      sprintf(buf, "%s tell you %s", clients[idx].name, msg);
      msg_send(clients[i].fd, buf);
      return;
    }
  }
  msg_send(clients[idx].fd, "ERROR: The receiver doesn't exist.");
}

void cmd_yell(struct Client *clients, int idx, int idx_bound, char *msg) {
  char buf[LINE_MAX];
  sprintf(buf, "%s yell %s", clients[idx].name, msg);
  msg_broadcast(clients, idx_bound, buf);
}

void cmd_who(struct Client *clients, int idx, int idx_bound) {
  for (int i = 0; i <= idx_bound; i++) {
    char buf[LINE_MAX];
    if (clients[i].fd >= 0) {
      sprintf(buf, "%s %s:%d", clients[idx].name, clients[idx].ip,
              clients[idx].port);
      if (i == idx)
        strcat(buf, " ->me");
      msg_send(clients[i].fd, buf);
    }
  }
}

void def_cmd(struct Client *clients, int idx, int idx_bound, char *cmd) {
  char *pos = strtok(cmd, " \n");
  if (!pos) {
    ;
  } else if (strcmp(pos, "who") == 0) {
    pos = strtok(NULL, "\n");
    if (!pos) {
      cmd_who(clients, idx, idx_bound);
      return;
    }
  } else if (strcmp(pos, "name") == 0) {
    pos = strtok(NULL, "\n");
    if (pos) {
      cmd_name(clients, idx, idx_bound, pos);
      return;
    }
  } else if (strcmp(pos, "yell") == 0) {
    pos = strtok(NULL, "\n");
    if (pos) {
      cmd_yell(clients, idx, idx_bound, pos);
      return;
    }
  } else if (strcmp(pos, "tell") == 0) {
    pos = strtok(NULL, " \n");
    if (pos) {
      char *m = strtok(NULL, "\n");
      if (m) {
        cmd_tell(clients, idx, idx_bound, pos, m);
        return;
      }
    }
  }
  msg_send(clients[idx].fd, "ERROR: Error command.");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <SERVER PORT>\n", argv[0]);
    exit(0);
  }

  int listen_fd;
  int idx_bound = 0;
  struct Client clients[CLIENT_MAX];
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

      inet_ntop(AF_INET, &cli_addr.sin_addr, clients[idx].ip,
                sizeof(clients[idx].ip));
      clients[idx].port = ntohs(cli_addr.sin_port);
      user_come(clients, idx);

      if (idx == CLIENT_MAX) {
        printf("Chat Room is full!\n");
        exit(0);
      }
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

    int idx = 0;
    for (; idx <= idx_bound && nready; idx++) {
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

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define LINE_MAX 1024
#define CLIENT_MAX 10

struct Client {
  int fd;
  char name[16];
  char ip[16];
  int port;
};

struct Client clients[CLIENT_MAX];

void send_msg(int fd, char *msg) {
  char buf[LINE_MAX];
  sprintf(buf, "[Server] %s\n", msg);
  write(fd, buf, strlen(buf));
}

void broadcast(struct Client *clients, int idx_bound, char *msg) {
  for (int i = 0; i <= idx_bound; i++)
    if (clients[i].fd >= 0)
      send_msg(clients[i].fd, msg);
}

void come(struct Client *clients, int idx, struct sockaddr_in *cli_addr) {
  char buf[LINE_MAX];
  strcpy(clients[idx].name, "anonymous");

  inet_ntop(AF_INET, &cli_addr->sin_addr, clients[idx].ip,
            sizeof(clients[idx].ip));
  clients[idx].port = ntohs(ntohs(cli_addr->sin_port));

  sprintf(buf, "Hello, anonymous! From: %s:%d", clients[idx].ip,
          clients[idx].port);
  send_msg(clients[idx].fd, buf);
}

void name(struct Client *clients, int idx, int idx_bound, char *name) {
  if (strcmp(name, "anonymous") == 0) {
    send_msg(clients[idx].fd, "ERROR: Username cannot be anonymous.");
    return;
  }
  for (int i = 0; i <= idx_bound; i++) {
    if (clients[i].fd >= 0 && i != idx &&
        strcmp(name, clients[idx].name) == 0) {
      char buf[LINE_MAX];
      sprintf(buf, "ERROR: %s has been used by others.", name);
      send_msg(clients[idx].fd, buf);
      return;
    }
  }
  int name_len = strlen(name);
  if (name_len < 2 || name_len > 12 ||
      strspn(name, "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz") !=
          name_len) {
    send_msg(clients[idx].fd,
             "ERROR: Username can only consists of 2~12 English letters.");
    return;
  }

  char buf[LINE_MAX];

  sprintf(buf, "You're now known as %s.", name);
  send_msg(clients[idx].fd, buf);

  int fd = clients[idx].fd;
  clients[idx].fd = -1;
  sprintf(buf, "%s is now known as %s.", clients[idx].name, name);
  broadcast(clients, idx_bound, buf);
  clients[idx].fd = fd;

  strcpy(clients[idx].name, name);
}

void tell(struct Client *clients, int idx, int idx_bound, char *name,
          char *msg) {
  if (strcmp(clients[idx].name, "anonymous") == 0) {
    send_msg(clients[idx].fd, "ERROR: You are anonymous.");
    return;
  }
  if (strcmp(name, "anonymous") == 0) {
    send_msg(clients[idx].fd,
             "ERROR: The client to which you sent is anonymous.");
    return;
  }
  for (int i = 0; i <= idx_bound; i++) {
    if (clients[i].fd >= 0 && strcmp(clients[i].name, name) == 0) {
      char buf[LINE_MAX];

      send_msg(clients[idx].fd, "SUCCESS: Your message has been sent.");

      sprintf(buf, "%s tell you %s", clients[idx].name, msg);
      send_msg(clients[i].fd, buf);
      return;
    }
  }
  send_msg(clients[idx].fd, "ERROR: The receiver doesn't exist.");
}

void yell(struct Client *clients, int idx, int idx_bound, char *msg) {
  char buf[LINE_MAX];
  sprintf(buf, "%s yell %s", clients[idx].name, msg);
  broadcast(clients, idx_bound, buf);
}

void who(struct Client *clients, int idx, int idx_bound) {
  for (int i = 0; i <= idx_bound; i++) {
    char buf[LINE_MAX];
    if (clients[i].fd >= 0) {
      sprintf(buf, "%s %s:%d", clients[idx].name, clients[idx].ip,
              clients[idx].port);
      if (i == idx)
        strcat(buf, " ->me");
      send_msg(clients[i].fd, buf);
    }
  }
}

void leave(struct Client *clients, int idx, int idx_bound) {
  char buf[LINE_MAX];
  sprintf(buf, "%s is offline.", clients[idx].name);
  broadcast(clients, idx_bound, buf);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <SERVER PORT>\n", argv[0]);
    exit(0);
  }
  return 0;
}

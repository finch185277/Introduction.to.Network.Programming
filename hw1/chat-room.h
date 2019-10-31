#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CLIENT_MAX 10
#define LINE_MAX 1024

struct Client {
  int fd;
  char name[16];
  char ip[16];
  int port;
};

void msg_unicast(int fd, char *msg);
void msg_broadcast(struct Client *clients, int idx_bound, char *msg);

void user_sign_in(struct Client *clients, int idx, int idx_bound, int cli_fd,
                  struct sockaddr_in *cli_addr);
void user_sign_out(struct Client *clients, int idx, int idx_bound);

void cmd_who(struct Client *clients, int idx, int idx_bound);
void cmd_name(struct Client *clients, int idx, int idx_bound, char *name);
void cmd_tell(struct Client *clients, int idx, int idx_bound, char *name,
              char *msg);
void cmd_yell(struct Client *clients, int idx, int idx_bound, char *msg);

void def_cmd(struct Client *clients, int idx, int idx_bound, char *cmd);

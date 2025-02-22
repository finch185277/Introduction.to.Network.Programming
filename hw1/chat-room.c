#include "chat-room.h"

void msg_unicast(int fd, char *msg) {
  char buf[LINE_MAX];
  sprintf(buf, "[Server] %s\n", msg);
  write(fd, buf, strlen(buf));
}

void msg_broadcast(struct Client *clients, int idx_bound, char *msg) {
  for (int i = 0; i <= idx_bound; i++)
    if (clients[i].fd >= 0)
      msg_unicast(clients[i].fd, msg);
}

void user_sign_in(struct Client *clients, int idx, int idx_bound, int cli_fd,
                  struct sockaddr_in *cli_addr) {
  char buf[LINE_MAX];
  msg_broadcast(clients, idx_bound, "Someone is coming!");

  clients[idx].fd = cli_fd;
  strcpy(clients[idx].name, "anonymous");
  inet_ntop(AF_INET, &cli_addr->sin_addr, clients[idx].ip, INET_ADDRSTRLEN);
  clients[idx].port = ntohs(cli_addr->sin_port);

  sprintf(buf, "Hello, anonymous! From: %s:%d", clients[idx].ip,
          clients[idx].port);
  msg_unicast(clients[idx].fd, buf);
}

void user_sign_out(struct Client *clients, int idx, int idx_bound) {
  char buf[LINE_MAX];
  clients[idx].fd = -1;
  sprintf(buf, "%s is offline.", clients[idx].name);
  msg_broadcast(clients, idx_bound, buf);
}

void cmd_who(struct Client *clients, int idx, int idx_bound) {
  for (int i = 0; i <= idx_bound; i++) {
    if (clients[i].fd >= 0) {
      char buf[LINE_MAX];
      if (i == idx)
        sprintf(buf, "%s %s:%d ->me", clients[idx].name, clients[idx].ip,
                clients[idx].port);
      else
        sprintf(buf, "%s %s:%d", clients[idx].name, clients[idx].ip,
                clients[idx].port);
      msg_unicast(clients[idx].fd, buf);
    }
  }
}

void cmd_name(struct Client *clients, int idx, int idx_bound, char *name) {
  if (strcmp(name, "anonymous") == 0) {
    msg_unicast(clients[idx].fd, "ERROR: Username cannot be anonymous.");
    return;
  }
  for (int i = 0; i <= idx_bound; i++) {
    if (clients[i].fd >= 0 && i != idx &&
        strcmp(name, clients[idx].name) == 0) {
      char buf[LINE_MAX];
      sprintf(buf, "ERROR: %s has been used by others.", name);
      msg_unicast(clients[idx].fd, buf);
      return;
    }
  }
  int name_len = strlen(name);
  if (name_len < CLIENT_NAME_MIN || name_len > CLIENT_NAME_MAX ||
      strspn(name, "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz") !=
          name_len) {
    char buf[LINE_MAX];
    sprintf(buf, "ERROR: Username can only consists of %d~%d English letters.",
            CLIENT_NAME_MIN, CLIENT_NAME_MAX);
    msg_unicast(clients[idx].fd, buf);
    return;
  }
  char buf[LINE_MAX];

  sprintf(buf, "You're now known as %s.", name);
  msg_unicast(clients[idx].fd, buf);

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
    msg_unicast(clients[idx].fd, "ERROR: You are anonymous.");
    return;
  }
  if (strcmp(name, "anonymous") == 0) {
    msg_unicast(clients[idx].fd,
                "ERROR: The client to which you sent is anonymous.");
    return;
  }
  for (int i = 0; i <= idx_bound; i++) {
    if (clients[i].fd >= 0 && strcmp(clients[i].name, name) == 0) {
      char buf[LINE_MAX];
      msg_unicast(clients[idx].fd, "SUCCESS: Your message has been sent.");
      sprintf(buf, "%s tell you %s", clients[idx].name, msg);
      msg_unicast(clients[i].fd, buf);
      return;
    }
  }
  msg_unicast(clients[idx].fd, "ERROR: The receiver doesn't exist.");
}

void cmd_yell(struct Client *clients, int idx, int idx_bound, char *msg) {
  char buf[LINE_MAX];
  sprintf(buf, "%s yell %s", clients[idx].name, msg);
  msg_broadcast(clients, idx_bound, buf);
}

void def_cmd(struct Client *clients, int idx, int idx_bound, char *cmd) {
  char *tok = strtok(cmd, " \n");
  if (!tok) {
    msg_unicast(clients[idx].fd, "ERROR: Error command.");
  } else if (strcmp(tok, "who") == 0) {
    tok = strtok(NULL, "\n");
    if (!tok) {
      cmd_who(clients, idx, idx_bound);
      return;
    }
  } else if (strcmp(tok, "name") == 0) {
    tok = strtok(NULL, "\n");
    if (tok) {
      cmd_name(clients, idx, idx_bound, tok);
      return;
    }
  } else if (strcmp(tok, "tell") == 0) {
    tok = strtok(NULL, " \n");
    if (tok) {
      char *msg = strtok(NULL, "\n");
      if (msg) {
        cmd_tell(clients, idx, idx_bound, tok, msg);
        return;
      }
    }
  } else if (strcmp(tok, "yell") == 0) {
    tok = strtok(NULL, "\n");
    if (tok) {
      cmd_yell(clients, idx, idx_bound, tok);
      return;
    }
  }
  msg_unicast(clients[idx].fd, "ERROR: Error command.");
}

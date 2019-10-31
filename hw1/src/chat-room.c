#include "chat-room.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
    if (clients[i].fd >= 0) {
      char buf[LINE_MAX];
      if (i == idx)
        sprintf(buf, "%s %s:%d ->me", clients[idx].name, clients[idx].ip,
                clients[idx].port);
      else
        sprintf(buf, "%s %s:%d", clients[idx].name, clients[idx].ip,
                clients[idx].port);
      msg_send(clients[idx].fd, buf);
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

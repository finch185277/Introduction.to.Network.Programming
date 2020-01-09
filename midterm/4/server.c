#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SEND_MAX 2048
#define HISTORY_MAX 1024
#define LINE_MAX 200
#define CLIENT_MAX 10
#define NATION_MAX 5
#define MONEY_MAX 6
#define CLIENT_NAME_MAX 5

struct Client {
  int fd;
  int money;
  char name[CLIENT_NAME_MAX];
};

void msg_send(int fd, char *msg) {
  char buf[SEND_MAX];
  sprintf(buf, "%s", msg);
  write(fd, buf, strlen(buf));
}

void cmd_history(struct Client *client) {
  char buf[SEND_MAX];
  memset(buf, 0, sizeof(buf));

  FILE *fp = fopen(client->name, "r");

  if (fp == NULL) {
    strcat(buf, "%%% No commands received so far\n");
    strcat(buf, "$$$ BALANCE: 0 NTD\n");
  } else {
    char history[HISTORY_MAX];
    int n = read(fileno(fp), history, HISTORY_MAX - 1);
    history[n] = '\0';
    strcat(buf, history);
  }

  msg_send(client->fd, buf);
}

void cmd_deposit(struct Client *client, char *money, char *nation) {
  int NTD = atoi(money);
  if (strcmp(nation, "USD") == 0) {
    NTD *= 30;
  }

  char buf[LINE_MAX];
  char command[LINE_MAX];
  memset(buf, 0, sizeof(buf));
  char history[HISTORY_MAX];
  memset(history, 0, sizeof(history));

  client->money += NTD;

  FILE *fp = fopen(client->name, "r");
  if (fp == NULL) {
    FILE *fp2 = fopen(client->name, "w");

    sprintf(command, "### DEPOSIT %s %s\n", money, nation);
    strcat(history, command);

    char stamp[LINE_MAX];
    sprintf(stamp, "$$$ BALANCE: %d NTD\n", client->money);
    strcat(history, stamp);

    write(fileno(fp2), history, strlen(history));
  } else {
    int m = read(fileno(fp), history, HISTORY_MAX);
    char *remain = strtok(history, "$");

    FILE *fp2 = fopen(client->name, "w");
    char history2[HISTORY_MAX];
    memset(history2, 0, sizeof(history2));
    strcat(history2, remain);

    sprintf(command, "### DEPOSIT %s %s\n", money, nation);
    strcat(history2, command);

    char stamp[LINE_MAX];
    sprintf(stamp, "$$$ BALANCE: %d NTD\n", client->money);
    strcat(history2, stamp);

    write(fileno(fp2), history2, strlen(history2));
  }

  char msg[LINE_MAX];
  sprintf(msg, "### BALANCE: %d NTD\n", client->money);
  strcat(buf, msg);
  msg_send(client->fd, buf);
}

void cmd_withdraw(struct Client *client, char *money, char *nation) {
  int NTD = atoi(money);
  if (strcmp(nation, "USD") == 0) {
    NTD *= 30;
  }

  char buf[LINE_MAX];
  char command[LINE_MAX];
  memset(buf, 0, sizeof(buf));
  char history[HISTORY_MAX];
  memset(history, 0, sizeof(history));

  if (client->money - NTD < 0) {
    strcat(buf, "!!! FAILED: Not enough money in the account\n");
    sprintf(command, "### WITHDRAW %s %s (FAILED)\n", money, nation);
  } else {
    client->money -= NTD;
    sprintf(command, "### WITHDRAW %s %s\n", money, nation);
  }

  FILE *fp = fopen(client->name, "r");
  if (fp == NULL) {
    FILE *fp2 = fopen(client->name, "w");

    strcat(history, command);

    char stamp[LINE_MAX];
    sprintf(stamp, "$$$ BALANCE: %d NTD\n", client->money);
    strcat(history, stamp);

    write(fileno(fp2), history, strlen(history));
  } else {
    int m = read(fileno(fp), history, HISTORY_MAX);
    char *remain = strtok(history, "$");

    FILE *fp2 = fopen(client->name, "w");
    char history2[HISTORY_MAX];
    memset(history2, 0, sizeof(history2));
    strcat(history2, remain);
    strcat(history2, command);

    char stamp[LINE_MAX];
    sprintf(stamp, "$$$ BALANCE: %d NTD\n", client->money);
    strcat(history2, stamp);

    write(fileno(fp2), history2, strlen(history2));
  }

  char msg[LINE_MAX];
  sprintf(msg, "### BALANCE: %d NTD\n", client->money);
  strcat(buf, msg);
  msg_send(client->fd, buf);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <SERVER PORT>\n", argv[0]);
    exit(0);
  }

  int idx_bound = 0;
  struct Client clients[CLIENT_MAX];
  for (int i = 0; i < CLIENT_MAX; i++)
    clients[i].fd = -1;

  struct sockaddr_in listen_addr;
  memset(&listen_addr, 0, sizeof(listen_addr));
  listen_addr.sin_family = AF_INET;
  listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  listen_addr.sin_port = htons(atoi(argv[1]));

  int listen_fd;
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);

  bind(listen_fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr));

  listen(listen_fd, CLIENT_MAX);

  fd_set new_set, r_set;
  FD_SET(listen_fd, &new_set);
  int max_fd = listen_fd + 1;

  for (;;) {
    r_set = new_set;
    int nready = select(max_fd, &r_set, NULL, NULL, NULL);

    if (FD_ISSET(listen_fd, &r_set)) {
      struct sockaddr_in cli_addr;
      int cli_len = sizeof(cli_addr);
      int cli_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_len);

      int idx = 0;
      for (; idx < CLIENT_MAX; idx++)
        if (clients[idx].fd < 0)
          break;

      if (idx == CLIENT_MAX) {
        msg_send(cli_fd, "Client count is full!");
        close(cli_fd);
      } else {
        clients[idx].fd = cli_fd;
        clients[idx].money = 0;

        if (idx > idx_bound)
          idx_bound = idx;
        if (cli_fd >= max_fd)
          max_fd = cli_fd + 1;
        FD_SET(cli_fd, &new_set);
      }
      nready--;
    }

    for (int idx = 0; idx <= idx_bound && nready >= 0; idx++) {
      if (FD_ISSET(clients[idx].fd, &r_set)) {
        char buf[LINE_MAX];
        int n = read(clients[idx].fd, buf, LINE_MAX - 1);
        if (n == 0) {
          close(clients[idx].fd);
          FD_CLR(clients[idx].fd, &new_set);
          clients[idx].fd = -1;
          continue;
        }
        buf[n] = '\0';
        char *tok = strtok(buf, " \n");
        if (tok == NULL) {
          ;
        } else if (strcmp(tok, "CLIENTID:") == 0) {
          tok = strtok(NULL, " \n");
          if (tok != NULL) {
            strcpy(clients[idx].name, tok);
            FILE *fp = fopen(clients[idx].name, "r");
            if (fp != NULL) {
              char history[HISTORY_MAX];
              int m = read(fileno(fp), history, HISTORY_MAX);
              int i = 0, j = 0;
              for (; i < m; i++)
                if (history[i] == ':')
                  break;
              for (j = i; j < strlen(history); j++)
                if (history[j] == 'N')
                  break;

              char money[MONEY_MAX];
              memset(money, 0, sizeof(money));
              int mid = 0;
              for (int k = i + 1; k < j; k++)
                money[mid++] = history[k];

              clients[idx].money = atoi(money);
            }
            continue;
          }
        } else if (strcmp(tok, "EXIT") == 0) {
          tok = strtok(NULL, "\n");
          if (tok == NULL) {
            close(clients[idx].fd);
            FD_CLR(clients[idx].fd, &new_set);
            clients[idx].fd = -1;
            continue;
          }
        } else if (strcmp(tok, "HISTORY") == 0) {
          tok = strtok(NULL, "\n");
          if (tok == NULL) {
            cmd_history(&clients[idx]);
            continue;
          }
        } else if (strcmp(tok, "DEPOSIT") == 0) {
          tok = strtok(NULL, " \n");
          if (tok != NULL) {
            char money[MONEY_MAX];
            strcpy(money, tok);
            tok = strtok(NULL, "\n");
            if (tok != NULL) {
              char nation[NATION_MAX];
              strcpy(nation, tok);
              cmd_deposit(&clients[idx], money, nation);
              continue;
            }
          }
        } else if (strcmp(tok, "WITHDRAW") == 0) {
          tok = strtok(NULL, " \n");
          if (tok != NULL) {
            char money[MONEY_MAX];
            strcpy(money, tok);
            tok = strtok(NULL, "\n");
            if (tok != NULL) {
              char nation[NATION_MAX];
              strcpy(nation, tok);
              cmd_withdraw(&clients[idx], money, nation);
              continue;
            }
          }
        }
        msg_send(clients[idx].fd, "ERROR: Error command.\n");
        nready--;
      }
    }
  }
  return 0;
}

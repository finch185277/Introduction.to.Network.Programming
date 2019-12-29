#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define LINE_MAX 1024
#include <string>
#include <unordered_map>
#include <unordered_set>

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <SERVER PORT>\n", argv[0]);
    exit(0);
  }

  struct sockaddr_in listen_addr;
  memset(&listen_addr, 0, sizeof(listen_addr));
  listen_addr.sin_family = AF_INET;
  listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  listen_addr.sin_port = htons(atoi(argv[1]));

  int listen_fd;
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);

  bind(listen_fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr));

  listen(listen_fd, 1024);

  int flag = fcntl(listen_fd, F_GETFL, 0);
  fcntl(listen_fd, F_SETFL, flag | O_NONBLOCK);

  fd_set new_set, r_set;
  FD_SET(listen_fd, &new_set);
  int max_fd = listen_fd + 1;

  std::unordered_map<std::string, std::unordered_set<int>> list;
  std::unordered_map<std::string, std::unordered_set<std::string>> dirs;

  for (;;) {
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int cli_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_len);

    if (cli_fd > 0) {
      char buf[LINE_MAX];
      int n = read(cli_fd, buf, LINE_MAX - 1);
      buf[n] = '\0';

      std::string name(buf);
      auto user = list.find(name);

      flag = fcntl(cli_fd, F_GETFL, 0);
      fcntl(cli_fd, F_SETFL, flag | O_NONBLOCK);

      if (user == list.end())
        list.insert(
            std::pair<std::string, std::unordered_set<int>>(name, {cli_fd}));
      else
        user->second.insert(cli_fd);
    }

    for (auto user : list) {
      for (auto cli = user.second.begin(); cli != user.second.end(); cli++) {
        char buf[LINE_MAX];
        int n = read(*cli, buf, LINE_MAX - 1);
        if (n > 0) {
          buf[n] = '\0';
          if (strcmp(buf, "exit\n") == 0) {
            close(*cli);
            user.second.erase(cli);
          } else {
            for (auto fd : user.second)
              write(fd, buf, n);
          }
        }
      }
    }
  }

  return 0;
}

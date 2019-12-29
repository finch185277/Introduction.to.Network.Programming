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
#include <vector>

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

  std::unordered_map<std::string, std::vector<int>> list;

  for (;;) {
    r_set = new_set;
    int nready = select(max_fd, &r_set, NULL, NULL, NULL);

    if (FD_ISSET(listen_fd, &r_set)) {
      struct sockaddr_in cli_addr;
      socklen_t cli_len = sizeof(cli_addr);
      int cli_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_len);

      char buf[LINE_MAX];
      int n = read(cli_fd, buf, LINE_MAX);

      std::string name(buf);
      auto client = list.find(name);

      if (client == list.end()) {
        list.insert(std::pair<std::string, std::vector<int>>(name, {cli_fd}));
      } else {
        client->second.emplace_back(cli_fd);
      }

      if (cli_fd >= max_fd)
        max_fd = cli_fd + 1;

      FD_SET(cli_fd, &new_set);
      nready--;
    }

    for (auto client : list) {
      for (auto fd = client.second.begin(); fd != client.second.end(); fd++) {
        if (FD_ISSET(*fd, &r_set)) {
          char buf[LINE_MAX];
          int n = read(*fd, buf, LINE_MAX);
          if (n != 0) {
            write(*fd, buf, n);
          } else {
            close(*fd);
            FD_CLR(*fd, &new_set);
            client.second.erase(fd);
          }
          nready--;
        }
      }
    }
  }

  return 0;
}

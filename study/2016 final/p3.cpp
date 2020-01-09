#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <unordered_map>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <server IP>\n", argv[0]);
    exit(0);
  }

  std::unordered_map<int, int> fd_ports;
  int max_fd = 0;

  for (int port = 10000; port <= 11000; port++) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    int flags = fcntl(sock_fd, F_GETFL, 0);
    fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in srv_addr;
    bzero(&srv_addr, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    inet_pton(AF_INET, argv[1], &srv_addr.sin_addr);

    int n =
        connect(sock_fd, (const struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if (n < 0) {
      if (errno == EINPROGRESS) {
        fd_ports.insert(std::pair<int, int>(sock_fd, port));
        max_fd = sock_fd > max_fd ? sock_fd : max_fd;
      }
    } else {
      printf("TCP port number %d is open.\n", port);
    }
  }

  for (;;) {
    fd_set fs;
    FD_ZERO(&fs);
    for (auto fd : fd_ports)
      FD_SET(fd.first, &fs);

    select(max_fd + 1, NULL, &fs, NULL, NULL);

    std::unordered_map<int, int> copy_fds(fd_ports);
    for (auto fd : copy_fds) {
      if (FD_ISSET(fd.first, &fs)) {
        int ret;
        socklen_t len;
        getsockopt(fd.first, SOL_SOCKET, SO_ERROR, &ret, &len);
        if (ret == 0)
          printf("TCP port number %d is open.\n", fd.second);
        fd_ports.erase(fd.first);
      }
    }

    if (fd_ports.empty())
      break;
  }

  return 0;
}

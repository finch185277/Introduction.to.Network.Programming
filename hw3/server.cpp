#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define LINE_MAX 1024
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

void send_file(int fd, std::string file_name) {
  char send_buf[LINE_MAX];
  std::ifstream infile;
  infile.open(file_name, std::ios_base::binary);

  const auto begin = infile.tellg();
  infile.seekg(0, std::ios_base::end);
  const auto end = infile.tellg();
  infile.seekg(0, std::ios_base::beg);
  int file_size = end - begin;

  // send file name
  sprintf(send_buf, "%s", file_name.c_str());
  write(fd, send_buf, sizeof(file_name));
  printf("send name: [%s]\n", file_name.c_str());

  // send file size
  sprintf(send_buf, "%d", file_size);
  write(fd, send_buf, sizeof(send_buf));
  printf("send size: %d\n", file_size);

  // send file content
  char *content = new char[file_size];
  infile.read(content, file_size);
  write(fd, content, file_size);
  printf("send content: %s\n", content);
  delete content;

  return;
}

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

      std::string user_name(buf);
      auto user = list.find(user_name);

      flag = fcntl(cli_fd, F_GETFL, 0);
      fcntl(cli_fd, F_SETFL, flag | O_NONBLOCK);

      if (user == list.end()) {
        // create directory for user
        mkdir(user_name.c_str(), 0755);

        // add new user
        list.insert(std::pair<std::string, std::unordered_set<int>>(user_name,
                                                                    {cli_fd}));
      } else {
        // sync client files
        auto dir = dirs.find(user_name);
        for (auto file_name : dir->second)
          send_file(cli_fd, user_name + "/" + file_name);

        // add client
        user->second.insert(cli_fd);
      }
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
          } else if (strcmp(buf, "put") == 0) {
            // get file name
            char file_name[LINE_MAX];
            n = read(*cli, file_name, LINE_MAX - 1);
            file_name[n] = '\0';
            printf("get file name: %s\n", file_name);

            // get file size
            char file_size[LINE_MAX];
            n = read(*cli, file_size, LINE_MAX - 1);
            file_size[n] = '\0';
            int size = atoi(file_size);
            printf("get file size: %d\n", size);

            // get file content
            char *content = new char[size];
            FILE *fp = fopen(file_name, "w+t");
            n = read(*cli, content, size);
            write(fileno(fp), content, size);
            fclose(fp);
            printf("get file content: %s\n", content);

            // record file name
            std::string file(file_name);
            auto dir = dirs.find(user.first);
            dir->second.insert(file);

            // sync with clients (same user)
            for (auto fd : user.second) {
              std::string file(file_name);
              send_file(fd, user.first + "/" + file);
            }
          }
        }
      }
    }
  }

  return 0;
}

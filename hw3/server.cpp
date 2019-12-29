#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define LINE_MAX 1060
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

struct segment_t {
  char seq_no[10];
  char action[10];
  char file_name[20];
  char file_size[10];
  char seg_size[10];
  char content[1000];
};

void send_file(int fd, std::string file_name) {
  std::ifstream infile;
  infile.open(file_name, std::ios_base::binary);

  const auto begin = infile.tellg();
  infile.seekg(0, std::ios_base::end);
  const auto end = infile.tellg();
  infile.seekg(0, std::ios_base::beg);
  int file_size = end - begin;

  int loops = (file_size / 1000) + 1;
  for (int i = 1; i <= loops; i++) {
    struct segment_t segment;
    sprintf(segment.seq_no, "%d", i);
    sprintf(segment.file_name, "%s", file_name.c_str());
    sprintf(segment.file_size, "%d", file_size);

    if (i != loops)
      infile.read(segment.content, 1000);
    else
      infile.read(segment.content, file_size % 1000);

    write(fd, &segment, sizeof(segment));
  }

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
          send_file(cli_fd, file_name);

        // add client
        user->second.insert(cli_fd);
      }
    }

    for (auto user : list) {
      for (auto cli = user.second.begin(); cli != user.second.end(); cli++) {
        struct segment_t segment;
        int n = read(*cli, &segment, sizeof(segment));
        if (n > 0) {
          if (strcmp(segment.action, "exit") == 0) {
            close(*cli);
            user.second.erase(cli);
          } else if (strcmp(segment.action, "put") == 0) {
            std::string file(segment.file_name);
            file = user.first + "/" + file;
            FILE *fp = fopen(file.c_str(), "w+t");

            write(fileno(fp), segment.content, atoi(segment.seg_size));

            int loops = (atoi(segment.file_size) / 1000) + 1;
            for (int i = 2; i <= loops; i++) {
              write(fileno(fp), segment.content, atoi(segment.seg_size));
            }

            fclose(fp);

            // record file name
            auto dir = dirs.find(user.first);
            dir->second.insert(file);

            // sync with clients (same user)
            for (auto fd : user.second) {
              send_file(fd, file);
            }
          }
        }
      }
    }
  }

  return 0;
}

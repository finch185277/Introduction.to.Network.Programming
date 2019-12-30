#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define LINE_MAX 1050
#define CONTENT_SIZE 1000
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

struct segment_t {
  char action[10];
  char file_name[20];
  char file_size[20];
  char content[1000];
};

struct proc_file_t {
  std::string file_name;
  int file_size;
  int already_read;
};

void send_file(int fd, std::string file_name) {
  std::ifstream infile;
  infile.open(file_name, std::ios_base::binary);

  const auto begin = infile.tellg();
  infile.seekg(0, std::ios_base::end);
  const auto end = infile.tellg();
  infile.seekg(0, std::ios_base::beg);
  int file_size = end - begin;

  struct segment_t segment;
  strcpy(segment.action, "put");
  sprintf(segment.file_name, "%s", file_name.c_str());
  sprintf(segment.file_size, "%d", file_size);
  write(fd, &segment, sizeof(segment));

  char *content = new char[file_size];
  infile.read(content, file_size);
  write(fd, content, file_size);
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

  // key: user name; value: fds
  std::unordered_map<std::string, std::unordered_set<int>> user_lists;
  // key: user name; value: files
  std::unordered_map<std::string, std::unordered_set<std::string>> user_files;
  // key: fd; value: files
  std::unordered_map<int, std::unordered_set<std::string>> fd_files;
  // key: fd; value: file name
  std::unordered_map<int, struct proc_file_t> proc_fds;

  for (;;) {
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int cli_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_len);

    if (cli_fd > 0) {
      char buf[LINE_MAX];
      int n = read(cli_fd, buf, LINE_MAX - 1);
      buf[n] = '\0';

      flag = fcntl(cli_fd, F_GETFL, 0);
      fcntl(cli_fd, F_SETFL, flag | O_NONBLOCK);

      std::string user_name(buf);
      auto user = user_lists.find(user_name);

      if (user == user_lists.end()) {
        // create directory for user
        mkdir(user_name.c_str(), 0755);

        // add new user
        user_lists.insert(std::pair<std::string, std::unordered_set<int>>(
            user_name, {cli_fd}));

        // init user files
        user_files.insert(
            std::pair<std::string, std::unordered_set<std::string>>(user_name,
                                                                    {}));
      } else {
        // sync client files
        // auto dir = dirs.find(user_name);
        // for (auto file_name : dir->second)
        //   send_file(cli_fd, file_name);

        // add new client
        user->second.insert(cli_fd);
      }

      // init fd files
      fd_files.insert(
          std::pair<int, std::unordered_set<std::string>>(cli_fd, {}));
    }

    for (auto user : user_lists) {
      std::string user_name(user.first);
      for (auto cli_fd : user.second) {
        auto fd = proc_fds.find(cli_fd);
        if (fd != proc_fds.end()) {
          std::string file_name = fd->second.file_name;
          int file_size = fd->second.file_size;
          int already_read = fd->second.already_read;
          int left = file_size - already_read;
          int read_size = (left > CONTENT_SIZE) ? CONTENT_SIZE : left;

          FILE *fp = fopen(file_name.c_str(), "a+t");
          char content[read_size];
          int n = read(cli_fd, content, read_size);
          if (n < 0)
            continue;
          write(fileno(fp), content, n);
          printf("write content %d bytes\n", n);
          fclose(fp);

          if (already_read + n == file_size) {
            proc_fds.erase(cli_fd);
            auto fd_file = fd_files.find(cli_fd);
            fd_file->second.insert(file_name);
            auto user_file = user_files.find(user_name);
            fd_file->second.insert(file_name);
          } else {
            fd->second.already_read += n;
            continue;
          }
        }

        struct segment_t segment;
        int n = read(cli_fd, &segment, sizeof(segment));
        if (n == sizeof(segment)) {
          if (strcmp(segment.action, "exit") == 0) {
            close(cli_fd);
            user.second.erase(cli_fd);
            fd_files.erase(cli_fd);
            proc_fds.erase(cli_fd);
          } else if (strcmp(segment.action, "put") == 0) {
            printf("wanna get file: %s\n", segment.file_name);

            // get file exact path
            std::string file_name(segment.file_name);
            std::string exact_file_name(user_name + "/" + file_name);
            printf("wanna open file: %s\n", exact_file_name.c_str());

            int file_size = atoi(segment.file_size);

            struct proc_file_t proc_file;
            proc_file.file_name = exact_file_name;
            proc_file.file_size = file_size;
            proc_file.already_read = 0;

            proc_fds.insert(
                std::pair<int, struct proc_file_t>(cli_fd, proc_file));
          }
        }
      }
    }
  }

  return 0;
}

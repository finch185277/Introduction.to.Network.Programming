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
};

struct proc_file_t {
  std::string file_name;
  int file_size;
  int already_read;
};

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
  std::unordered_map<int, struct proc_file_t> upload_fds;
  // key: fd; value: file name
  std::unordered_map<int, struct proc_file_t> download_fds;

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
        // client downloading file
        auto down_fd = download_fds.find(cli_fd);
        if (down_fd != download_fds.end()) {
          std::string file_name(down_fd->second.file_name);
          std::string abs_file_name(user_name + "/" + file_name);
          int file_size = down_fd->second.file_size;
          int already_read = down_fd->second.already_read;
          int left = file_size - already_read;
          int read_size = (left > CONTENT_SIZE) ? CONTENT_SIZE : left;

          FILE *fp = fopen(abs_file_name.c_str(), "r");
          fseek(fp, already_read, SEEK_SET);
          char content[read_size];
          int n = read(fileno(fp), content, read_size);
          if (n < 0)
            continue;
          write(cli_fd, content, n);
          // printf("already sync %d bytes\n", already_read + n);
          fclose(fp);

          if (already_read + n == file_size) {
            download_fds.erase(cli_fd);
            auto fd_file = fd_files.find(cli_fd);
            fd_file->second.insert(file_name);
          } else {
            down_fd->second.already_read += n;
            continue;
          }
        }

        // sync client files
        auto user_file = user_files.find(user_name);
        auto fd_file = fd_files.find(cli_fd);
        for (auto file_name : user_file->second) {
          if (fd_file->second.count(file_name) == 0) {
            // printf("file: %s need sync!\n", file_name.c_str());
            // get file size
            std::ifstream infile;
            infile.open(user_name + "/" + file_name, std::ios_base::binary);
            const auto begin = infile.tellg();
            infile.seekg(0, std::ios_base::end);
            const auto end = infile.tellg();
            infile.seekg(0, std::ios_base::beg);
            int file_size = end - begin;

            // send control message
            struct segment_t segment;
            strcpy(segment.action, "sync");
            sprintf(segment.file_name, "%s", file_name.c_str());
            sprintf(segment.file_size, "%d", file_size);
            int n = write(cli_fd, &segment, sizeof(segment));
            // printf("send segment %d bytes\n", n);

            struct proc_file_t proc_file;
            proc_file.file_name = file_name;
            proc_file.file_size = file_size;
            proc_file.already_read = 0;

            download_fds.insert(
                std::pair<int, struct proc_file_t>(cli_fd, proc_file));

            continue;
          }
        }

        // client uploading file
        auto up_fd = upload_fds.find(cli_fd);
        if (up_fd != upload_fds.end()) {
          std::string file_name(up_fd->second.file_name);
          std::string abs_file_name(user_name + "/" + file_name);
          int file_size = up_fd->second.file_size;
          int already_read = up_fd->second.already_read;
          int left = file_size - already_read;
          int read_size = (left > CONTENT_SIZE) ? CONTENT_SIZE : left;

          FILE *fp = fopen(abs_file_name.c_str(), "a+t");
          char content[read_size];
          int n = read(cli_fd, content, read_size);
          if (n < 0)
            continue;
          write(fileno(fp), content, n);
          // printf("write content %d bytes\n", n);
          fclose(fp);

          if (already_read + n == file_size) {
            upload_fds.erase(cli_fd);
            auto fd_file = fd_files.find(cli_fd);
            fd_file->second.insert(file_name);
            auto user_file = user_files.find(user_name);
            user_file->second.insert(file_name);
          } else {
            up_fd->second.already_read += n;
            continue;
          }
        }

        // receive control message
        struct segment_t segment;
        int n = read(cli_fd, &segment, sizeof(segment));
        if (n == sizeof(segment)) {
          if (strcmp(segment.action, "exit") == 0) {
            close(cli_fd);
            user.second.erase(cli_fd);
            fd_files.erase(cli_fd);
            upload_fds.erase(cli_fd);
          } else if (strcmp(segment.action, "put") == 0) {
            // printf("wanna get file: %s\n", segment.file_name);

            // get file absolute path
            std::string file_name(segment.file_name);
            std::string abs_file_name(user_name + "/" + file_name);
            // printf("wanna open file: %s\n", abs_file_name.c_str());

            int file_size = atoi(segment.file_size);

            // open new file
            FILE *fp = fopen(abs_file_name.c_str(), "w+t");
            fclose(fp);

            struct proc_file_t proc_file;
            proc_file.file_name = file_name;
            proc_file.file_size = file_size;
            proc_file.already_read = 0;

            upload_fds.insert(
                std::pair<int, struct proc_file_t>(cli_fd, proc_file));
          }
        }
      }
    }
    continue;
  }

  return 0;
}

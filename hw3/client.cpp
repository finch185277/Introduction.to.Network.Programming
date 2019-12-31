#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define LINE_MAX 1050
#define CONTENT_SIZE 1000
#define PROC_BAR_LEN 20
#include <fstream>
#include <string>

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

std::string cal_proc_bar(int file_size, int nread) {
  std::string proc_bar;
  int proc_num = PROC_BAR_LEN * nread / file_size;
  for (int i = 0; i < PROC_BAR_LEN; i++) {
    if (i < proc_num)
      proc_bar += "#";
    else
      proc_bar += " ";
  }
  return proc_bar;
}

int main(int argc, char **argv) {
  if (argc != 4) {
    printf("Usage: %s <SERVER IP> <SERVER PORT> <USERNAME>\n", argv[0]);
    exit(0);
  }

  printf("Welcome to the dropbox-like server: %s\n", argv[3]);

  struct sockaddr_in srv_addr;
  memset(&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(atoi(argv[2]));

  struct hostent *host = gethostbyname(argv[1]);
  memcpy(&srv_addr.sin_addr, host->h_addr, host->h_length);

  int sock_fd;
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);

  int flag = fcntl(sock_fd, F_GETFL, 0);
  fcntl(sock_fd, F_SETFL, flag | O_NONBLOCK);

  flag = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flag | O_NONBLOCK);

  connect(sock_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
  write(sock_fd, argv[3], sizeof(argv[3]));

  std::pair<bool, struct proc_file_t> upload_stat;
  std::pair<bool, struct proc_file_t> download_stat;

  int pid = getpid();

  for (;;) {
    // read from stdin
    char buf[LINE_MAX];
    int n = read(STDIN_FILENO, buf, LINE_MAX - 1);
    if (n > 0) {
      buf[n] = '\0';
      if (strcmp(buf, "exit\n") == 0) {
        write(sock_fd, buf, n);
        close(sock_fd);
        break;
      }
      char *tok = strtok(buf, " \n");
      if (strcmp(tok, "put") == 0) {
        // get file name
        char file_name[LINE_MAX];
        tok = strtok(NULL, " \n");
        strcpy(file_name, tok);

        // get file size
        std::ifstream infile;
        infile.open(file_name, std::ios_base::binary);
        const auto begin = infile.tellg();
        infile.seekg(0, std::ios_base::end);
        const auto end = infile.tellg();
        infile.seekg(0, std::ios_base::beg);
        int file_size = end - begin;

        // send control message
        struct segment_t segment;
        strcpy(segment.action, "put");
        sprintf(segment.file_name, "%s", file_name);
        sprintf(segment.file_size, "%d", file_size);
        int n = write(sock_fd, &segment, sizeof(segment));

        // record upload stat
        struct proc_file_t proc_file;
        proc_file.file_name = std::string(file_name);
        proc_file.file_size = file_size;
        proc_file.already_read = 0;
        upload_stat.first = true;
        upload_stat.second = proc_file;

        printf("Pid: %d [Upload] %s Start!\n", pid, file_name);

      } else if (strcmp(tok, "sleep") == 0) {
        tok = strtok(NULL, " \n");
        int sec = atoi(tok);
        printf("Pid: %d The client starts to sleep.\n", pid);
        for (int i = 0; i < sec; i++) {
          printf("Pid: %d Sleep %d\n", pid, i);
          sleep(1);
        }
      }
    }

    // download file
    if (download_stat.first) {
      std::string file_name = download_stat.second.file_name;
      int file_size = download_stat.second.file_size;
      int already_read = download_stat.second.already_read;
      int left = file_size - already_read;
      int read_size = (left > CONTENT_SIZE) ? CONTENT_SIZE : left;

      char content[read_size];
      int n = read(sock_fd, content, read_size);
      if (n < 0)
        continue;
      if (n == 0) {
        close(sock_fd);
        break;
      }
      FILE *fp = fopen(file_name.c_str(), "a+t");
      write(fileno(fp), content, n);
      fclose(fp);

      if (already_read + n == file_size) {
        download_stat.first = false;
        printf("Pid: %d Progress : [######################]\n", pid);
        printf("Pid: %d [Download] %s Finish!\n", pid, file_name.c_str());
      } else {
        std::string proc_bar = cal_proc_bar(file_size, already_read + n);
        printf("Pid: %d Progress : [%s]\r", pid, proc_bar.c_str());
        download_stat.second.already_read += n;
      }

      continue;
    }

    // check sync
    struct segment_t segment;
    n = read(sock_fd, &segment, sizeof(segment));
    if (n == sizeof(segment)) {
      if (strcmp(segment.action, "sync") == 0) {
        std::string file_name(segment.file_name);
        int file_size = atoi(segment.file_size);

        // open new file
        FILE *fp = fopen(file_name.c_str(), "w+t");
        fclose(fp);

        // record download stat
        struct proc_file_t proc_file;
        proc_file.file_name = file_name;
        proc_file.file_size = file_size;
        proc_file.already_read = 0;
        download_stat.first = true;
        download_stat.second = proc_file;

        printf("Pid: %d [Download] %s Start!\n", pid, file_name.c_str());

        continue;
      }
    } else if (n == 0) {
      close(sock_fd);
      break;
    }

    // upload file
    if (upload_stat.first) {
      std::string file_name = upload_stat.second.file_name;
      int file_size = upload_stat.second.file_size;
      int already_read = upload_stat.second.already_read;
      int left = file_size - already_read;
      int read_size = (left > CONTENT_SIZE) ? CONTENT_SIZE : left;

      FILE *fp = fopen(file_name.c_str(), "r");
      fseek(fp, already_read, SEEK_SET);
      char content[read_size];
      int n = read(fileno(fp), content, read_size);
      if (n < 0)
        continue;
      write(sock_fd, content, n);
      fclose(fp);

      if (already_read + n == file_size) {
        upload_stat.first = false;
        printf("Pid: %d Progress : [######################]\n", pid);
        printf("Pid: %d [Upload] %s Finish!\n", pid, file_name.c_str());
      } else {
        std::string proc_bar = cal_proc_bar(file_size, already_read + n);
        printf("Pid: %d Progress : [%s]\r", pid, proc_bar.c_str());
        upload_stat.second.already_read += n;
      }
    }
    continue;
  }

  return 0;
}

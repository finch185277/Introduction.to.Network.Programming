// client
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUF_SIZE 1000

struct segment_t {
  long int seq_no;
  long int length;
  char data[BUF_SIZE];
};

int readable_select_timeout(int fd, int sec, int usec) {
  fd_set r_set;
  FD_ZERO(&r_set);
  FD_SET(fd, &r_set);

  struct timeval tv;
  tv.tv_sec = sec;
  tv.tv_usec = usec;

  return select(fd + 1, &r_set, NULL, NULL, &tv);
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Usage: %s [send filename] [target address] [connect port]\n",
           argv[0]);
    exit(0);
  }

  struct hostent *host = gethostbyname(argv[2]);

  struct sockaddr_in dst_addr;
  memset(&dst_addr, 0, sizeof(dst_addr));
  dst_addr.sin_family = AF_INET;
  memcpy(&dst_addr.sin_addr, host->h_addr, host->h_length);
  dst_addr.sin_port = htons(atoi(argv[3]));

  int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

  if (access(argv[1], R_OK) == 0) {
    // get file size
    struct stat st;
    stat(argv[1], &st);
    long int file_size = st.st_size;

    // init timeout setting
    int expect_timeout = 1, repeat_timeout_counter = 0;

    // calculate total segment amount
    long int total_seg = 0, ack_no = 0;
    if (file_size % BUF_SIZE == 0) {
      total_seg = file_size / BUF_SIZE;
    } else {
      total_seg = file_size / BUF_SIZE + 1;
    }

    struct sockaddr_in src_addr;
    int sock_len = sizeof(src_addr);

    // send total segment amount
    while (ack_no != total_seg) {
      struct segment_t segment;
      segment.seq_no = 0;
      sprintf(segment.data, "%ld", total_seg);
      sendto(sock_fd, &segment, sizeof(segment), 0,
             (struct sockaddr *)&dst_addr, sizeof(dst_addr));

      // recvfrom with select
      if (readable_select_timeout(sock_fd, expect_timeout, 0) == 0) {
        printf("socket timeout\n");
        repeat_timeout_counter++;
        if (repeat_timeout_counter == 10) {
          expect_timeout *= 2;
          repeat_timeout_counter = 0;
        }
        continue;
      } else {
        recvfrom(sock_fd, &ack_no, sizeof(ack_no), 0,
                 (struct sockaddr *)&src_addr, (socklen_t *)&sock_len);
        repeat_timeout_counter = 0;
      }
    }

    // send file
    FILE *file = fopen(argv[1], "r");
    for (long int idx = 1; idx <= total_seg;) {
      struct segment_t segment;
      segment.seq_no = idx;
      segment.length = fread(segment.data, sizeof(char), BUF_SIZE, file);

      sendto(sock_fd, &segment, sizeof(segment), 0,
             (struct sockaddr *)&dst_addr, sizeof(dst_addr));

      // recvfrom with select
      if (readable_select_timeout(sock_fd, expect_timeout, 0) == 0) {
        printf("socket timeout\n");
        repeat_timeout_counter++;
        if (repeat_timeout_counter == 10) {
          expect_timeout *= 2;
          repeat_timeout_counter = 0;
        }
        continue;
      } else {
        recvfrom(sock_fd, &ack_no, sizeof(ack_no), 0,
                 (struct sockaddr *)&src_addr, (socklen_t *)&sock_len);
        repeat_timeout_counter = 0;
      }

      while (ack_no != segment.seq_no) {
        sendto(sock_fd, &segment, sizeof(segment), 0,
               (struct sockaddr *)&dst_addr, sizeof(dst_addr));

        // recvfrom with select
        if (readable_select_timeout(sock_fd, expect_timeout, 0) == 0) {
          printf("socket timeout\n");
          repeat_timeout_counter++;
          if (repeat_timeout_counter == 10) {
            expect_timeout *= 2;
            repeat_timeout_counter = 0;
          }
          continue;
        } else {
          recvfrom(sock_fd, &ack_no, sizeof(ack_no), 0,
                   (struct sockaddr *)&src_addr, (socklen_t *)&sock_len);
          repeat_timeout_counter = 0;
        }
      }

      idx++;

      if (idx == total_seg + 1) {
        printf("File transfer success!\n");
      }
    }
    fclose(file);
  }
  return 0;
}

// server
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define BUF_SIZE 1000

struct segment_t {
  int seq_no;
  int length;
  char data[BUF_SIZE];
};

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s [save filename] [bind port]\n", argv[0]);
    exit(0);
  }

  struct sockaddr_in listen_addr;
  memset(&listen_addr, 0, sizeof(listen_addr));
  listen_addr.sin_family = AF_INET;
  listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  listen_addr.sin_port = htons(atoi(argv[2]));

  int listen_fd = socket(AF_INET, SOCK_DGRAM, 0);

  bind(listen_fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr));

  int total_seg = 0, bytes_recv = 0;

  struct sockaddr_in peer_addr;
  int sock_len = sizeof(peer_addr);

  // get total segment amount
  for (;;) {
    struct segment_t segment;

    recvfrom(listen_fd, &segment, sizeof(segment), 0,
             (struct sockaddr *)&peer_addr, (socklen_t *)&sock_len);

    if (segment.seq_no == 0)
      total_seg = atoi(segment.data);

    sendto(listen_fd, &total_seg, sizeof(total_seg), 0,
           (struct sockaddr *)&peer_addr, sizeof(peer_addr));

    if (segment.seq_no == 0)
      break;
  }

  // get file
  FILE *file = fopen(argv[1], "w+t");
  for (int idx = 1; idx <= total_seg;) {
    struct segment_t segment;

    recvfrom(listen_fd, &segment, sizeof(segment), 0,
             (struct sockaddr *)&peer_addr, (socklen_t *)&sock_len);

    if (segment.seq_no == 0) {
      sendto(listen_fd, &total_seg, sizeof(total_seg), 0,
             (struct sockaddr *)&peer_addr, sizeof(peer_addr));
    } else {
      sendto(listen_fd, &segment.seq_no, sizeof(segment.seq_no), 0,
             (struct sockaddr *)&peer_addr, sizeof(peer_addr));
    }

    if (segment.seq_no == idx) {
      fwrite(segment.data, sizeof(char), segment.length, file);
      bytes_recv += segment.length;
      printf("Bytes total received %d\n", bytes_recv);
      idx++;
    }

    if (idx == total_seg + 1) {
      printf("File transfer success!\n");
    }
  }
  fclose(file);

  return 0;
}

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char *sockaddr2str(const struct sockaddr *sa, int flags) {
  char host[128];
  char serv[16];

  socklen_t salen = sizeof(struct sockaddr_in);
  int n = getnameinfo(sa, salen, host, sizeof(host), serv, sizeof(serv), flags);

  if (n != 0)
    return NULL;

  char *ret = malloc(200 * sizeof(char));
  sprintf(ret, "%s", host);

  return ret;
}

int print_host(const char *header, const char *hostname) {
  struct addrinfo *result = NULL;
  struct addrinfo hint;

  memset(&hint, 0, sizeof(hint));
  hint.ai_socktype = SOCK_STREAM;
  int n = getaddrinfo(hostname, NULL, &hint, &result);

  if (n == 0) {
    struct addrinfo *cur = result;
    unsigned cnt = 0;

    printf("%-10s %s\n\n", header, hostname);
    while (cur) {
      char *dotted, *revhost;
      dotted = sockaddr2str(cur->ai_addr, NI_NUMERICHOST | NI_NUMERICSERV);
      revhost = sockaddr2str(cur->ai_addr, NI_NAMEREQD | NI_NUMERICSERV);

      if (dotted) {
        printf("Address %u: %s %c", ++cnt, dotted, revhost ? ' ' : '\n');
        if (revhost) {
          puts(revhost);
          free(revhost);
        }
        free(dotted);
      }

      cur = cur->ai_next;
    }
  } else {
    printf("can't resolve '%s'\n", hostname);
  }

  freeaddrinfo(result);
  return n;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <IP address>\n", argv[0]);
    exit(0);
  }

  print_host("Name:", argv[1]);

  return 0;
}

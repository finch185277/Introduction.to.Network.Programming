## socket
### socket
- BSD socket
```
struct sockaddr { /* only used to cast pointers */
  uint8_t sa_len;
  sa_family sa_family; /* address family: AF_xxx value */
  char sa_data[14];    /* protocol-specific address */
};
```
- internet socket
```
struct sockaddr_in {
  uint8_t sin_len;         /* length of structure */
  sa_family_t sin_family;  /* AF_INET */
  in_port_t sin_port;      /* 16-bit port#, network byte order */
  struct in_addr sin_addr; /* 32-bit IPv4 address, network byte order */
  char sin_zero[8];        /* unused */
};
```

### byte ordering functions

```
#include <netinet/in.h>
uint16_t htons(uint16_t host16bitvalue); // returns: value in network byte order
uint32_t htonl(uint32_t host32bitvalue); // returns: value in network byte order
uint16_t ntohs(uint16_t net16bitvalue);  // returns: value in host byte order
uint32_t ntohl(uint32_t net32bitvalue);  // returns: value in host byte order
```

### address conversion functions
- For IPv4 only: ascii and numeric

```
#include <arpa/inet.h>

int inet_aton(const char *strptr, struct in_addr *addrptr);
// returns: 1 if string is valid, 0 on error

int_addr_t inet_addr(const char *strptr);
// returns: 32-bit binary IPv4 addr, INADDR_NONE if error

char *inet_ntoa(struct in_addr inaddr);
// returns: pointer to dotted-decimal string
```
- For IPv4 (AF_INET) and IPv6 (AF_INET6): presentation and numeric

```
#include <arpa/inet.h>

INET_ADDRSTRLEN = 16 (for IPv4), INET6_ADDRSTRLEN = 46 (for IPv6 hex string)

int inet_pton(int family, const char *strptr, void *addrptr);
// returns: 1 if OK, 0 if invalid presentation, -1 on error

const char *inet_ntop(int family, const void *addrptr, char *strptr,
                      size_t len);
// returns: pointer to result if OK, NULL on error
```
## signal
### signal handler
- main function

```
Signal(SIGCHLD, sig_chld);
```
- handler

```
void sig_chld(int signo) {
  pid_t pid;
  int stat;

  while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
    printf("child %d terminated\n", pid);
  return;
}
```

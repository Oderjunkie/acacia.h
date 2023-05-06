#ifndef OTESUNKI_ACACIA_H
#define OTESUNKI_ACACIA_H
#define OTESUNKI_ACACIA_VER 0,1,0
  #include <unistd.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <setjmp.h>
  #include <arpa/inet.h>
  
  extern FILE *netin;
  extern FILE *netout;
  
  enum HttpMethodAcacia {
    UNKAH,
    GETAH, POSTAH, PUTAH, DELETEAH,
    CONNECTAH, OPTIONSAH, TRACEAH, PATCHAH
  };
  
  struct ConnectionAcacia {
    int sock;
    struct sockaddr_in host_addr;
    enum HttpMethodAcacia method;
    char *path;
  };
  
  #include <stdio.h>
  #include <stdlib.h>
  #include <errno.h>
  #include <sys/socket.h>
  #include <string.h>
  
  FILE *netin = NULL;
  FILE *netout = NULL;
  
  static int netfd = -1;
  static char *netbuf = NULL;
  static size_t netlen = 0;
  
  static inline int matchrequest_acacia(
    enum HttpMethodAcacia method,
    char path[],
    struct ConnectionAcacia *connection
  ) {
    if (connection->method != method)
      return 0;
    
    char *matcher, *matched;
    for (
      matcher = path, matched = connection->path;
      *matcher && *matched;
      matcher++, matched++
    ) {
      if (*matcher == '*') {
        if (matcher[1] == '*' && matcher[2] == '\0')
          return 1;
        while (*matched != '/' && *matched != '\0')
          matched++;
        matched--;
      } else if (*matcher != *matched)
        break;
    }
    
    return *matcher == *matched;
  }
  
  static inline void getnextrequest_acacia(struct ConnectionAcacia *connection) {
    static int netinfd = -1;
    static socklen_t size = sizeof(connection->host_addr);
    
    if ((netfd = accept(connection->sock, (struct sockaddr *) &connection->host_addr, &size)) < 0)
      fprintf(stderr, "\x1b[31m[ACACIA x00] Failed to accept incoming connection! (%s)\x1b[0m\n", strerror(errno)),
      exit(EXIT_FAILURE);
    
    if ((netinfd = dup(netfd)) == -1)
      fprintf(stderr, "\x1b[31m[ACACIA x01] Failed to duplicate network file descriptor! (%s)\x1b[0m\n", strerror(errno)),
      exit(EXIT_FAILURE);
    
    if ((netin = fdopen(netinfd, "r")) == NULL)
      fprintf(stderr, "\x1b[31m[ACACIA x02] Failed to `fdopen`! (%s)\x1b[0m\n", strerror(errno)),
      exit(EXIT_FAILURE);
    
    if ((netout = open_memstream(&netbuf, &netlen)) == NULL)
      fprintf(stderr, "\x1b[31m[ACACIA x07] Failed to turn buffer into file descriptor! (%s)\x1b[0m\n", strerror(errno)),
      exit(EXIT_FAILURE);
  }
  
  static inline void initresponse_acacia(struct ConnectionAcacia *connection) {
    char buf[256] = { '\0' };
    char reqmth[32] = { '\0' };
    char *reqpth = malloc(128);
    int majver = -1;
    int minver = -1;
    memset(reqpth, 0x00, 128);
    
    if (fgets(buf, sizeof(buf), netin) == NULL)
      fprintf(stderr, "\x1b[31m[ACACIA x0c] Failed to read line from browser's request! (%s)\x1b[0m\n", strerror(errno)),
      exit(EXIT_FAILURE);
    
    if (sscanf(buf, "%32s %128s HTTP/%d.%d", reqmth, reqpth, &majver, &minver) == 0)
      fprintf(stderr, "\x1b[31m[ACACIA x0c] Failed to parse browser's request! (%s)\x1b[0m\n", strerror(errno)),
      exit(EXIT_FAILURE);
    
    if (!strcmp(reqmth, "GET"))
      connection->method = GETAH;
    else
      connection->method = UNKAH;
    connection->path = reqpth;
  }
  
  static inline void sendresponse_acacia(struct ConnectionAcacia *connection) {
    FILE *rawnetout;
    
    if (fclose(netout) == EOF)
      fprintf(stderr, "\x1b[31m[ACACIA x0b] Failed to close `netout`! (netout = %p, %s)\x1b[0m\n", (void *) netout, strerror(errno)),
      exit(EXIT_FAILURE);
    
    if (fclose(netin) == -1)
      fprintf(stderr, "\x1b[31m[ACACIA x08] Failed to close `netin`! (netin = %p, %s)\x1b[0m\n", (void *) netin, strerror(errno)),
      exit(EXIT_FAILURE);
    
    if ((rawnetout = fdopen(netfd, "w")) == NULL)
      fprintf(stderr, "\x1b[31m[ACACIA x09] Failed to `fdopen`! (fd = %d, %s)\x1b[0m\n", netfd, strerror(errno)),
      exit(EXIT_FAILURE);
    
    fprintf(rawnetout,
      "HTTP/1.0 200 OK\n"
      "Server: acacia-c\n"
      "Content-type: text/html\n"
      "Content-Length: %d\n"
      "\n"
      "%s", (int) strlen(netbuf), netbuf);
    
    if (fclose(rawnetout) == -1)
      fprintf(stderr, "\x1b[31m[ACACIA x0a] Failed to close file! (file = %p, %s)\x1b[0m\n", (void *) rawnetout, strerror(errno)),
      exit(EXIT_FAILURE);
  
    if (connection->path)
      free(connection->path);
    free(netbuf);
    exit(EXIT_SUCCESS);
  }
  
  static inline struct ConnectionAcacia *port2con_acacia(struct ConnectionAcacia *connection, unsigned short port) {
    if ((connection->sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
      fprintf(stderr, "\x1b[31m[ACACIA x04] Failed to open socket! (%s)\x1b[0m\n", strerror(errno)),
      exit(EXIT_FAILURE);
    
    connection->host_addr.sin_family = AF_INET;
    connection->host_addr.sin_port = htons(port);
    connection->host_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(connection->sock, (struct sockaddr *) &connection->host_addr, sizeof(connection->host_addr)) != 0)
      fprintf(stderr, "\x1b[31m[ACACIA x05] Failed to bind socket to host address! (%s)\x1b[0m\n", strerror(errno)),
      exit(EXIT_FAILURE);
    
    if (listen(connection->sock, SOMAXCONN) != 0)
      fprintf(stderr, "\x1b[31m[ACACIA x06] Failed to listen to socket! (%s)\x1b[0m\n", strerror(errno)),
      exit(EXIT_FAILURE);
  
    return connection;
  }
  
  #define listen(port) \
    for ( \
      struct ConnectionAcacia *connection = port2con_acacia(&((struct ConnectionAcacia) {0}), (port)); \
      getnextrequest_acacia(connection), 1; \
      \
    ) \
    if (fork() != 0) (void) 0; else \
    for (initresponse_acacia(connection);;sendresponse_acacia(connection)) \
  
  #define route \
    for (int serve = 1; serve; serve = 0) \
    for (jmp_buf _serve_else, _serve_end; serve; serve = 0) \
    if (setjmp(_serve_else) == 0) \
    if (setjmp(_serve_end) != 0) (void) 0; else \
    for (;;longjmp(_serve_else, 1))
  
  #define GET(path) HTTP(GETAH, path)
  #define POST(path) HTTP(POSTAH, path)
  #define PUT(path) HTTP(PUTAH, path)
  #define DELETE(path) HTTP(DELETEAH, path)
  #define CONNECT(path) HTTP(CONNECTAH, path)
  #define OPTIONS(path) HTTP(OPTIONSAH, path)
  #define TRACE(path) HTTP(TRACEAH, path)
  #define PATCH(path) HTTP(PATCHAH, path)
  #define HTTP(method, path) \
    if (!matchrequest_acacia((method), path, connection)) (void) 0; else \
    for (;;longjmp(_serve_end, 1)) \
    switch (0) case 0
  
  #define netfd
  #define netbuf
  #define netlen
#endif

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

#define BACKLOG 20
#define MSGSIZE 2048
#define MAX_PATH_LENGTH 1023

int sockfd;
int client_fd;

// ./web_server 9001 /home/berg2007/Desktop/4061/P3/CSCI-4061--Project-3/P3/p3/testing 1 1 0 100 1

/**********************************************
 * init
   - port is the number of the port you want the server to be
     started on
   - initializes the connection acception/handling system
   - YOU MUST CALL THIS EXACTLY ONCE (not once per thread,
     but exactly one time, in the main thread of your program)
     BEFORE USING ANY OF THE FUNCTIONS BELOW
   - if init encounters any errors, it will call exit().
************************************************/
void init(int port) {
  struct sockaddr_in my_addr;
  int enable = 1;

  // create the socket
  if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    printf("Can't create socket\n");
    exit(1);
  }

  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(port);
  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // allows us to reuse the port number
  if (setsockopt(sockfd,SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
    printf("Can't set socket option\n");
    exit(1);
  }

  if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1) {
    printf("Could not bind\n");
    exit(1);
  }

  /* --- Set up the socket to listen for incoming connection requests --- */
  if (listen(sockfd, BACKLOG) == -1) {
    printf("Could not listen\n");
    exit(1);
  }

}

/**********************************************
 * accept_connection - takes no parameters
   - returns a file descriptor for further request processing.
     DO NOT use the file descriptor on your own -- use
     get_request() instead.
   - if the return value is negative, the request should be ignored.
***********************************************/
int accept_connection(void) {

  struct sockaddr_in client_addr;
  unsigned int addr_size;
  addr_size = sizeof(client_addr);

  // accept the connection
  if ((client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size)) == -1) {
    printf("Failed to accept connection\n");
    return -1;
  }
  fprintf(stderr, "\n\nserver: client connection from %s\n", inet_ntoa(client_addr.sin_addr));
  return client_fd;
}

/**********************************************
 * get_request
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        from where you wish to get a request
      - filename is the location of a character buffer in which
        this function should store the requested filename. (Buffer
        should be of size 1024 bytes.)
   - returns 0 on success, nonzero on failure. You must account
     for failures because some connections might send faulty
     requests. This is a recoverable error - you must not exit
     inside the thread that called get_request. After an error, you
     must NOT use a return_request or return_error function for that
     specific 'connection'.
************************************************/
int get_request(int fd, char *filename) {
  char msg[MSGSIZE];
  char type[10];
  char path[MSGSIZE];
  int nread = 0;

  // get message from client
  if ((nread = read(client_fd, msg, MSGSIZE-1)) >= 0) {
    msg[nread] = '\0';
    fprintf(stderr, "received from client: %s\n", msg);
  } else {
    printf("server read problem\n");
    return 1;
  }

  // parse the message and place the first two strings in type and path
  sscanf(msg, "%s %s", type, path);

  // make sure the type of the request is a "GET"
  if (strcmp(type, "GET") != 0) {
    printf("not a GET\n");
    return 1;
  }

  // there must be a space if there are at least 2 strings
  if (strstr(path," ") != NULL) {
    printf("needs to be at least 2 strings\n");
    return 1;
  }

  printf("path: %s\n", path);

  // check to see if the message contains either ".." or "//"
  if (strstr(path,"..") != NULL) {
    printf("found a ..\n");
    return 1;
  }
  if (strstr(path,"//") != NULL) {
    printf("found a //\n");
    return 1;
  }

  // make sure the string length isnt too long
  if (strlen(path) > MAX_PATH_LENGTH) {
    printf("path is too long\n");
    return 1;
  }

  // if everything is good, copy the path from the client to the output parameter and return
  strcpy(filename, path);

  // return 0 on success
  return 0;
}

/**********************************************
 * return_result
   - returns the contents of a file to the requesting client
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        to where you wish to return the result of a request
      - content_type is a pointer to a string that indicates the
        type of content being returned. possible types include
        "text/html", "text/plain", "image/gif", "image/jpeg" cor-
        responding to .html, .txt, .gif, .jpg files.
      - buf is a pointer to a memory location where the requested
        file has been read into memory (the heap). return_result
        will use this memory location to return the result to the
        user. (remember to use -D_REENTRANT for CFLAGS.) you may
        safely deallocate the memory after the call to
        return_result (if it will not be cached).
      - numbytes is the number of bytes the file takes up in buf
   - returns 0 on success, nonzero on failure.
************************************************/
int return_result(int fd, char *content_type, char *buf, int numbytes) {
  char good_request[MSGSIZE];

  // creates the string to be sent back to the client for a bad request
  sprintf(good_request,
    "HTTP/1.1 200 OK\nContent-Type: %s\nContent-Length: %d\nConnection: Close\n\n%s\n",
                              content_type,           numbytes,                 buf);
    //printf("returning:\n%s\n", good_request);

  // return 1 on failure, 0 on success
  if (write(fd, good_request, strlen(good_request)) == -1) {
    return 1;
  }
  return 0;
}

/**********************************************
 * return_error
   - returns an error message in response to a bad request
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        to where you wish to return the error
      - buf is a pointer to the location of the error text
   - returns 0 on success, nonzero on failure.
************************************************/
int return_error(int fd, char *buf) {
  char bad_request[MSGSIZE];

  // creates the string to be sent back to the client for a bad request
  sprintf(bad_request,
    "HTTP/1.1 404 Not Found\nContent-Type: Text/html\nContent-Length: %ld\nConnection: Close\n\n%s\n",
                                                                    strlen(buf),              buf);

  // return 1 on failure, 0 on success
  if (write(fd, bad_request, strlen(bad_request)) == -1) {
    return 1;
  }
  return 0;
}

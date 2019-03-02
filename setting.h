#ifndef SETTING_H
#define SETTING_H
#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

typedef struct t_playerlist playerlist;
struct t_playerlist{
  int fd;
  char hostname[50];
  char port[6];
};

typedef struct t_init_server init_server;
struct t_init_server{
  int fd;
  int port;
};

typedef struct t_neighbor_fd neighbor_fd;
struct t_neighbor_fd{
  int fd_left;
  int fd_right;
};

int bind_port(const char *port);
init_server initial_server(char * port,int nums_client);
int initial_client(const char * port,const char *hostname);
#endif

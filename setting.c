#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "setting.h"
int bind_port(const char *port){
  struct addrinfo *host_info_list;
  struct addrinfo host_info;
  const char *hostname = INADDR_ANY;
  int socket_fd;
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family   = AF_UNSPEC;// AF_INET :Ipv4 INET6: Ipv6
  host_info.ai_socktype = SOCK_STREAM;//TCP:stream
  host_info.ai_flags    =  AI_CANONNAME;

  if(getaddrinfo("0.0.0.0", port, &host_info, &host_info_list)!=0)
    return -1;

  socket_fd = socket(host_info_list->ai_family,
                     host_info_list->ai_socktype,
                     host_info_list->ai_protocol);

  int optname = 1;
  if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optname, sizeof(optname))==-1)
    return -1;
  if(bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen)==-1){
    freeaddrinfo(host_info_list);
    close(socket_fd);
    return -1;
  }
  else{
    freeaddrinfo(host_info_list);
    return socket_fd;
  }
}
init_server initial_server(char * port,int nums_client){
  int socket_fd=-1;
  init_server in_server;
 
  char port_try[6];
  if(port==NULL){
    int i=50000;
    while(i<80000&&socket_fd==-1){
      snprintf(port_try,6,"%d",i);
      socket_fd=bind_port(port_try);
      if(socket_fd!=-1){
	in_server.port=i;
	break;
      }
      i++;
     }
  }
  else{
    socket_fd=bind_port(port);
  }
  if (socket_fd == -1) {
    printf( "Error: cannot create socket.\n");
    exit(EXIT_FAILURE);
  }
  if(listen(socket_fd, nums_client)== -1) {
    printf("error: %s\n", strerror(errno)); 
    printf("Cannot listen on socket.\n");
    exit(EXIT_FAILURE);
  }
  in_server.fd=socket_fd;
  return in_server;
}

int initial_client(const char * port,const char *hostname){
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;


  if (getaddrinfo(hostname, port, &host_info, &host_info_list) != 0) {
    printf("Cannot get address info for host\n");
    exit(EXIT_FAILURE);
  } 

  socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    printf("Cannot create socket\n");
    exit(EXIT_FAILURE);
  }
 
  if (connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen) == -1) {
    freeaddrinfo(host_info_list);
    close(socket_fd);
    printf("Cannot connect to socket as client\n");
    exit(EXIT_FAILURE);
  }
  freeaddrinfo(host_info_list);
  return socket_fd;
}

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <time.h>
#include "setting.h"
#include "potato.h"
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

neighbor_fd connect_neigh(msg_playerinfo *playerinfo, init_server in_server){
  neighbor_fd  neigh_fd;
  //connect with neighbour
  //connect to right one as server,connect to left one as client
  int num=playerinfo->num_players;
  
  neigh_fd.fd_right=initial_client(playerinfo->port_connect,playerinfo->hostname_connect);
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  neigh_fd.fd_left=accept(in_server.fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (neigh_fd.fd_right == -1) {
    printf("Cannot accept connection on socket.\n");
    exit(EXIT_FAILURE);
  }
  return neigh_fd;
}
int main(int argc,const char *argv[]){
  if(argc!=3){
    printf("wrong number of arguements.\n");
    exit(EXIT_FAILURE);
  }
  const char * hostname=argv[1];
  const char * port_num=argv[2];
 
  int player_fd=initial_client(port_num,hostname);
    
  //get own hostname and send to ringmaster
  char myhostname[50];
  memset(&myhostname,0,50);
  if(gethostname(myhostname,50)==-1){
    printf("gethostname error.\n");
    exit(EXIT_FAILURE);
  }
  if(send(player_fd, myhostname,50, 0)<0){
    printf("send error\n");
    exit(EXIT_FAILURE);
  }
  //initial as a server and send port to ringmaster
  init_server in_server=initial_server(NULL,100);
  
  char myport[6];
  memset(&myport,0,50);
  snprintf(myport,6,"%d",in_server.port);
  // printf("myport is :%d\n",in_server.port);
  if(send(player_fd, myport,6, 0)<0){
    printf("send error\n");
    exit(EXIT_FAILURE);
  }
  
  //get information from ringmaster
   msg_playerinfo* playerinfo=malloc(sizeof(msg_playerinfo));
   memset(playerinfo,0,sizeof(msg_playerinfo));
  if(recv(player_fd, (char *)playerinfo, (int)sizeof(msg_playerinfo), 0)<0){
    printf("read error.\n");
    free(playerinfo);
    exit(EXIT_FAILURE);
  }
  if(playerinfo->type==PLAYERINFO){
    printf("Connected as player %d out of %d total players\n",playerinfo->id,playerinfo->num_players);
  }

  
  msg_connect_other *connect_other=malloc(sizeof(msg_connect_other));
  if(recv(player_fd, (char *)connect_other, (int)sizeof(msg_connect_other), 0)<0){
    printf("read error.\n");
    exit(EXIT_FAILURE);
  }
  if(connect_other->type==CONNECT_OTHER){
    printf("canconnect\n");
  }

  neighbor_fd neigh_fd;
  //connect to neighbor
  neigh_fd=connect_neigh(playerinfo,in_server);
   
  //ready to play
  msg_ready *readyplay=malloc(sizeof(msg_ready));
  memset(readyplay,0,sizeof(msg_ready));
  readyplay->type=PLAYER_READY;
  readyplay->id=playerinfo->id;
  send(player_fd,readyplay,sizeof(msg_ready), 0);

  srand((unsigned int)time(NULL)+playerinfo->id);
  int fd_list[3]={neigh_fd.fd_right,neigh_fd.fd_left,player_fd};
  //potato *mypotato=malloc(sizeof(potato));
  char mypotato[5000];
  //  memset(mypotato, 0, sizeof(potato));
  while(1){
    fd_set read_fd;
    FD_ZERO(&read_fd);
    int max_fd=fd_list[0];
    for(int i=0;i<3;i++){
      int temp=fd_list[i];
      if(temp>max_fd) max_fd=temp;
      FD_SET (temp, &read_fd);
    }
   
    int active=select(max_fd+1,&read_fd,NULL,NULL,NULL);
    if(active>0){
      for(int i=0;i<3;i++){
        if(FD_ISSET(fd_list[i], &read_fd)){
		//receive the potato
	        memset(mypotato, 0, sizeof(mypotato));
		recv(fd_list[i],mypotato,sizeof(mypotato), 0);
		if(mypotato->type==GAME_OVER){
                  free(mypotato);
                  exit(EXIT_SUCCESS);
          }
		char potato_remain[4];
		strcpy(potato_remain,mypotato);
		
		//if(mypotato->type==IS_POTATO){
		//  mypotato->track[mypotato->track_size]=playerinfo->id;
		  
		// mypotato->track_size++;
		//mypotato->hops_remain--;
		  //printf("hops remain:%d\n", mypotato->hops_remain);
		  if(mypotato->hops_remain>0){
		    //send potato to neighbor
                    int random = rand() % 2;
                    int chosen= random==0?playerinfo->id+1:playerinfo->id-1;
                    if(chosen==playerinfo->num_players)
                      chosen=0;
                    if(chosen==-1)
                      chosen=playerinfo->num_players-1;
		    printf("Sending potato to %d\n",chosen);
		    send(fd_list[random], (char *)mypotato,sizeof(potato), 0);
		  }
		  else{
		    printf("I’m it and my id is:%d\n",playerinfo->id);
		    send(player_fd,(char *)mypotato,sizeof(potato),0);
		   }
		  //}
		// }		
        }
      }
    }
  }  
  //free fd
  for(int i=0;i<3;i++){
    close(fd_list[i]);
  }
  return 0;
  
}

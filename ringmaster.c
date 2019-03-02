#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>
#include <time.h>
#include "setting.h"
#include "potato.h"
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>

int main(int argc,const char *argv[]){
  if(argc!=4){
    printf("wrong number of arguements.\n");
    exit(EXIT_FAILURE);
  }
  const char * port_num=argv[1];
  int num_players=atoi(argv[2]);
  if(num_players<=1){
    printf("you need at least 2 player.\n");
    exit(EXIT_FAILURE);
  }
  int num_hops=atoi(argv[3]);
  if(num_hops<0||num_hops>512){
    printf("wrong number of hops.\n");
    exit(EXIT_FAILURE);
  }
  printf("Potato Ringmaster\n");
  printf("Players = %d\n",num_players);
  printf("Hops = %d\n",num_hops);
  
  srand((unsigned int)time(NULL)+num_players);

  init_server in_server=initial_server((char *)port_num,num_players);
  int ringmaster_fd=in_server.fd;
  int *players_fd =  malloc(sizeof(int) * num_players);

  //receive palyers,get info from players
  playerlist *player_list=malloc(num_players*sizeof(playerlist));
  memset(player_list,0,num_players*sizeof(playerlist));
  for(int i=0;i<num_players;i++){
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    players_fd[i]= accept(ringmaster_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (players_fd[i] == -1) {
      printf("Cannot accept connection on socket.\n");
      exit(EXIT_FAILURE);
   } 
  
    //receive info from player
    recv(players_fd[i], player_list[i].hostname, 50, 0);
    recv(players_fd[i], player_list[i].port, 6, 0);
    player_list[i].fd=players_fd[i];
    
  }
  //send message to players
  for(int i=0;i<num_players;i++){
    msg_playerinfo *playerinfo=malloc(sizeof(msg_playerinfo));
    memset(playerinfo,0,sizeof(msg_playerinfo));
    playerinfo->type=PLAYERINFO;
    playerinfo->id=i;
    playerinfo->num_players=num_players;
    if(i==num_players-1){
      strcpy(playerinfo->port_connect,player_list[0].port);
      strcpy(playerinfo->hostname_connect,player_list[0].hostname);
    }
    else{ 
      strcpy(playerinfo->port_connect,player_list[i+1].port);
      strcpy(playerinfo->hostname_connect,player_list[i+1].hostname);
    }
    send(players_fd[i], (const char *)playerinfo, sizeof(msg_playerinfo), 0);
    free(playerinfo);
  }
  for(int i=0;i<num_players;i++){
    msg_connect_other *connect_other=malloc(sizeof(msg_connect_other));
    connect_other->type=CONNECT_OTHER;
    send(players_fd[i], (const char *)connect_other, sizeof(msg_connect_other), 0);
    free(connect_other);
  }

  int ready=0;
  while(1){
    fd_set read_fd;
    FD_ZERO(&read_fd);
    int max_fd=players_fd[0];
    for(int i=0;i<num_players;i++){
      int temp=players_fd[i];
      if(temp>max_fd) max_fd=temp;
      FD_SET (temp, &read_fd);
    }
    
    int active=select(max_fd+1,&read_fd,NULL,NULL,NULL);
    if(active>0){
      for(int i=0;i<num_players;i++){
        if(FD_ISSET(players_fd[i], &read_fd)){
	  //printf("%s\n",buffer);
	  if(ready<num_players){
		  msg_ready *readyplay=malloc(sizeof(msg_ready));
		  recv(players_fd[i], readyplay,sizeof(msg_ready), 0);
	          if(readyplay->type==PLAYER_READY){
	            printf("Player %d is ready to play\n",readyplay->id);
	            ready++;
	            if(ready==num_players){
		            //send potato
		      if(num_hops!=0){
		            int random = rand() % num_players;
			    printf("Ready to start the game, sending potato to player %d\n",random);
			    /*potato *mypotato=malloc(sizeof(potato));
			    memset(mypotato, 0, sizeof(potato));
			    mypotato->type=IS_POTATO;
			    mypotato->hops_remain=num_hops;
			    mypotato->track_size=0;
			    send(players_fd[random], (char *)mypotato,sizeof(potato), 0);*/
			    char mypotato[5000];
			    memset(&mypotato,0,sizeof(mypotato));
			    snprintf(mypotato,4,"%d",num_hops);
			    send(players_fd[random], (char *)mypotato,sizeof(mypotato), 0);
		      }
		      else{
			msg_end *end=malloc(sizeof(msg_end));char mypotato[5000];
                            memset(&mypotato,0,sizeof(mypotato));
                            snprintf(mypotato,4,"%d",num_hops);
                            send(players_fd[random], (char *)mypotato,sizeof(mypotato), 0);
			memset(end, 0, sizeof(msg_end));
			end->type=GAME_OVER;
			for(int i=0;i<num_players;i++){
			  send(players_fd[i], (char *)end,sizeof(msg_end), 0);
			}
			exit(EXIT_SUCCESS);
		      }
		    }  
		  }
		  free(readyplay);
		}
		else{
		  potato *potato_back=malloc(sizeof(potato));
		  memset(potato_back, 0, sizeof(potato));
		  recv(players_fd[i], potato_back,sizeof(potato), 0);
		  if(potato_back->type==IS_POTATO){
		    printf("Trace of potato:\n");
		    for(int i=0;i<num_hops-1;i++){
		      printf("%d,",potato_back->track[i]);
		    }
		    printf("%d",potato_back->track[num_hops-1]);
		    msg_end *end=malloc(sizeof(msg_end));
		    end->type=GAME_OVER;
		    for(int i=0;i<num_players;i++){
		      send(players_fd[i], (char *)end,sizeof(msg_end), 0);
		    }
		    free(player_list);
		    free(potato_back);
		    exit(EXIT_SUCCESS);
		  }
		}
        }
      }
    }
   }
  //free fd
  for(int i=0;i<num_players;i++){
    close(players_fd[i]);
  }
  close(ringmaster_fd);
}

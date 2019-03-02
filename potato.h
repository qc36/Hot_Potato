#ifndef POTATO_H
#define POTATO_H
#include<stdio.h>
#include<stdlib.h>



enum msg_type
{
    PLAYERINFO,
    PLAYER_READY,
    IS_POTATO,
    GAME_OVER,
    AS_SERVER,
    CONNECT_OTHER,
};

typedef struct t_msg_playerinfo msg_playerinfo;
struct t_msg_playerinfo{
  enum msg_type type;
  int id;
  int num_players;
  char port_connect[6];
  char hostname_connect[50];
};

typedef struct t_msg_ready msg_ready;
struct t_msg_ready{
  enum msg_type type;
  int id;
};

typedef struct t_msg_as_server msg_as_server;
struct t_msg_as_server{
  enum msg_type type;
};

typedef struct t_msg_connect_other msg_connect_other;
struct t_msg_connect_other{
  enum msg_type type;
};

typedef struct t_potato potato;
struct t_potato{
  enum msg_type type;
  int hops_remain;
  int track_size;
  int track[512];
};
typedef struct t_msg_end msg_end;
struct t_msg_end{
  enum msg_type type;
};
#endif

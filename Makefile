GFLAGS==-ggdb3 -Wall -Werror -std=gnu99 -pedantic
all: ringmaster player

player:player.c setting.h setting.c
	gcc $(CFLAGS) -o player player.c setting.h setting.c
ringmaster: ringmaster.c setting.h setting.c
	gcc $(CFLAGS) -o ringmaster ringmaster.c setting.h setting.c

clean:
	rm -f *.o *~ player ringmaster


#include <irmo.h>

#include "universe.h"

#define SAMPLE_TIME 50

long long get_ms()
{
	struct timeval nowtime;

	gettimeofday(&nowtime, NULL);

	return nowtime.tv_sec * 1000 + nowtime.tv_usec / 1000;
}

int main(int argc, char *argv[])
{
	long long oldmovetime;
	int i;

	srand(time(NULL));
	
	universe_init();
	server_init();

	for (i=0; i<7; ++i)
		universe_new_rock();
	
	for (oldmovetime = get_ms();; ) {
		long long nowtime = get_ms();
		
		if (nowtime - oldmovetime > SAMPLE_TIME) {
			universe_run();
			oldmovetime += SAMPLE_TIME;
		} else
			usleep(SAMPLE_TIME / 4);
		
		server_run(); 
	}
}

// $Log: not supported by cvs2svn $

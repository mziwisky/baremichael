
/* 
 * Copyright 2010 Carsten Clauss, Chair for Operating Systems,
 *                                RWTH Aachen University
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifdef RCCE_SUPPORT

#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <scc.h>

#include "RCCE.h"


#define MAXBUFSIZE  1024*1024*4
#define DEFAULTLEN  (1024*1024)
#define NUMROUNDS   10000


#define send_buffer buffer
#define recv_buffer buffer
static char buffer[MAXBUFSIZE+1];

int RCCE_pingpong(void);

int RCCE_pingpong()
{
	int num_ranks;
	int remote_rank, my_rank;
	int numrounds = NUMROUNDS;
	int maxlen    = DEFAULTLEN;
	int length;
	int round;
	unsigned long long timer;
	unsigned int corefreq = get_tile_freq(get_my_tileid());

	// fake argc and argv w/ rccerun-type args
	// name, nue, clockspeed, list of coreids
	int argc = 5;
	char *args[] = {"RCCE_pingpong", "2", "0.533", "0", "1"};
	char **argv = args;

	RCCE_init(&argc, &argv);

	my_rank   = RCCE_ue();
	num_ranks = RCCE_num_ues();

	remote_rank = (my_rank + 1) % 2;

	if(my_rank == 0) printf("\n[RCCE PING-PONG]\n");
	if(my_rank == 0) printf("#bytes,upperTicks,lowerTicks,#rounds,corefreq\n");

	for(length=1; length <= maxlen; length*=2)
	{

		/* synchronize before starting PING-PONG: */
		RCCE_barrier(&RCCE_COMM_WORLD);

		for(round=0; round < numrounds+1; round++)
		{

			if(my_rank == 0)
			{
				/* send PING via pipelined blocking send: */
				RCCE_send(send_buffer, length, remote_rank);
				/* recv PONG via pipelined blocking recv: */
				RCCE_recv(recv_buffer, length, remote_rank);
			} 
			else
			{
				/* recv PING via pipelined blocking recv: */
				RCCE_recv(recv_buffer, length, remote_rank);
				/* send PONG via pipelined blocking send: */
				RCCE_send(send_buffer, length, remote_rank);
			}

			/* start timer: */
			if(round == 0) timer = getticks();

		}

		/* stop timer: */
		timer = getticks() - timer;
		ulong ticks = (ulong)timer;
		ulong upperTicks = (ulong)(timer >> 32);

		if (my_rank == 0) {
			/* print results */
			printf("%u,%u,%u,%u,%u\n",
					length, upperTicks, ticks, numrounds, corefreq);
		}
	}
	if (my_rank == 0) { printf("That's all, folks!\n"); }

	RCCE_finalize();

	return 0;
}
#endif /* RCCE_SUPPORT */

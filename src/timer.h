#include <stdio.h>        /* printf */
#include <stdlib.h>       /* EXIT_SUCCESS */
#include <stdint.h>       /* int32_t uint32_t */
#include <pthread.h>      /* pthread_t pthread_create pthread_join */
#include <semaphore.h>    /* sem_t sem_init sem_wait sem_post */
#include <errno.h>        /* errno */
#include <signal.h>       /* struct sigevent */
#include <sys/neutrino.h> /* ChannelCreate ConnectAttach MsgReceive */
#include <sys/netmgr.h>   /* ND_LOCAL_NODE */
#include <time.h>         /* struct itimerspec struct timespec
                             timer_create tier_settime clock_gettime */

//durée de la simulation en seconde
#define MAX_EXECUTION_TIME 10

/* Pulse code definition */
#define TASK_PULSE_CODE _PULSE_CODE_MINAVAIL

/* Thread arguments structure */
typedef struct thread_arg {
	sem_t*   semaphore; /* Synchronization semaphore pointer */
	uint32_t id;        /* Task id */
	uint32_t starttime; /* Global start time */
	int32_t  chid;      /* Task channel id */
	int run = 1;
} thread_args_t;

/* Pulse dumy structure */
typedef union pulse_msg{
	struct _pulse pulse;
} pulse_msg_t;

/******************************************************************************
 * Timer initialization routine
 * The function will initialize a timer given the parameters.
 *****************************************************************************/
int32_t init_timer(struct sigevent* event, struct itimerspec* itime,
		           timer_t* timer, const int32_t chanel_id,
				   const uint32_t period) {
	int32_t error;
	int32_t period_s;
	int32_t period_ns;

	/* Set event as pulse and attach to channel */
	event->sigev_notify = SIGEV_PULSE;
	event->sigev_coid   = ConnectAttach(ND_LOCAL_NODE, 0, chanel_id,
									    _NTO_SIDE_CHANNEL, 0);
	/* Set basic priority and set event code */
	event->sigev_priority = 0;
	event->sigev_code     = TASK_PULSE_CODE;

	/* Create timer and associate to event */
	error = timer_create(CLOCK_MONOTONIC, event, timer);
	if(0 != error) {
		printf("Error creating timer\n");
		return error;
	}

	/* Set the itime structure */
	period_s  = period / 1000;
	period_ns = (1000000 * period) - (period_s * 1000000000);
	itime->it_value.tv_sec = period_s;
	itime->it_value.tv_nsec = period_ns;
	itime->it_interval.tv_sec = period_s;
	itime->it_interval.tv_nsec = period_ns;

	/* Set the timer period */
	return timer_settime(*timer, 0, itime, NULL);
}



/******************************************************************************
 * Task pulse handler routine
 * Handles a pulse and release the semaphore.
 *****************************************************************************/
void* task_pulse_handler(void* args) {

	sem_t*      sync_sem;
	int32_t     rcvid;
	pulse_msg_t msg;
	int32_t     task_chid;

	/* Get the arguments */
	sync_sem  = ((thread_args_t*)args)->semaphore;
	task_chid = ((thread_args_t*)args)->chid;


	while(((thread_args_t*)args)->run==1) {
		/* Get the pulse message */
		rcvid = MsgReceive(task_chid, &msg, sizeof(pulse_msg_t), NULL);
		if (0 == rcvid) {
			if (TASK_PULSE_CODE == msg.pulse.code) {
				/* Release semaphore */
				if(0 != sem_post(sync_sem)) {
					/* Print error */
					printf("Could not post semaphore: %d\n", errno);
				}
			}
			else {
				/* Print error */
				printf("Unknown message received: %d\n", rcvid);
			}
		}
		else {
			/* Print error */
			printf("Message receive failed: %d (%d)\n", rcvid, errno);
		}
	}
}


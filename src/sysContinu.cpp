#include "sysContinu.h"


void* run_continu (void* args)
{
	continious_t system = *(continious_t*)args;
	pthread_mutex_lock(mtx_lvl_batterie);
	*(system.batterie.)	= ;
	pthread_mutex_unlock(mtx_lvl_batterie);
}

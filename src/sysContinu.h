#ifndef __CONTINU_H_
#define __CONTINU_H_

/**********************************************************************************************
 * 
 *  Modélisation du systeme continu
 * 
 * ********************************************************************************************/

#include <cstdint>   /* Generic types */
#include <pthread.h> /* pthread_mutex_t, pthread_cond_t */
#include <iostream>

#include "automobile.h"
#include "Navigation.h"
#include "Batterie.h"


typedef struct continious{
	// declaration des mutex
	// input
	pthread_mutex_t mtx_pos;
	pthread_mutex_t mtx_vitesse;
	pthread_mutex_t mtx_orientation;
	pthread_mutex_t mtx_lvl_batterie;
	// output
	pthread_mutex_t mtx_ctrl_charge;
	pthread_mutex_t mtx_ctrl_vitesse;
	pthread_mutex_t mtx_ctrl_orientation;

} continious_t;

#endif

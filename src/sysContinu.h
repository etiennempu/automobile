#ifndef __CONTINU_H_
#define __CONTINU_H_

/**********************************************************************************************
 * 
 *  Modélisation du systeme continu
 * 
 * ********************************************************************************************/

// #include <cstdint>   /* Generic types */
#include <pthread.h> /* pthread_mutex_t, pthread_cond_t */
#include <iostream>


typedef struct continious{
	// input
	pthread_mutex_t mtx_pos;
	pthread_mutex_t mtx_vitesse;
	pthread_mutex_t mtx_orientation;
	pthread_mutex_t mtx_lvl_batterie;
	// output
	pthread_mutex_t mtx_ctrl_charge;
	pthread_mutex_t mtx_ctrl_vitesse;
	pthread_mutex_t mtx_ctrl_orientation;
    // alarm 10
    pthread_mutex_t mtx_alarm10;
    pthread_mutex_t mtx_alarm80;

} continious_t;



extern continious_t globalmutex;
// Fonction d'initialisation de la structure
void initContinious_t(continious_t *data);



#endif

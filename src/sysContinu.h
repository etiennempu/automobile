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

// Fonction d'initialisation de la structure
void initContinious_t(continious_t *data) {
    if (pthread_mutex_init(&data->mtx_pos, NULL) != 0) {
        perror("Erreur d'initialisation du mutex pos");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&data->mtx_vitesse, NULL) != 0) {
        perror("Erreur d'initialisation du mutex vitesse");
        exit(EXIT_FAILURE);
    }
	 if (pthread_mutex_init(&data->mtx_orientation, NULL) != 0) {
        perror("Erreur d'initialisation du mutex orientation");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&data->mtx_lvl_batterie, NULL) != 0) {
        perror("Erreur d'initialisation du mutex lvl batterie");
        exit(EXIT_FAILURE);
    }
	 if (pthread_mutex_init(&data->mtx_ctrl_charge, NULL) != 0) {
        perror("Erreur d'initialisation du mutex ctrl  charge");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&data->mtx_ctrl_vitesse, NULL) != 0) {
        perror("Erreur d'initialisation du mutex ctrl vitesse");
        exit(EXIT_FAILURE);
    }
	 if (pthread_mutex_init(&data->mtx_ctrl_orientation, NULL) != 0) {
        perror("Erreur d'initialisation du mutex ctrl orientation");
        exit(EXIT_FAILURE);
    }
}

extern continious_t globalmutex;

#endif

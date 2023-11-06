#include "sysContinu.h"


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
    if (pthread_mutex_init(&data->mtx_alarm10, NULL) != 0) {
        perror("Erreur d'initialisation du mutex alarm10");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&data->mtx_alarm80, NULL) != 0) {
        perror("Erreur d'initialisation du mutex alarm80");
        exit(EXIT_FAILURE);
    }
}

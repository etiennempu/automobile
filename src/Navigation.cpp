#include "Navigation.h"



void Navigation::compute_orientation()
{
	// lire la consigne d'orientation
	pthread_mutex_lock(&globalmutex.mtx_ctrl_orientation);
	float ctrl_orientation_buffer = ctrl_orientation;
	pthread_mutex_unlock(&globalmutex.mtx_ctrl_orientation);

    //  symbolise le fait que la voiture dévie naturellement de son axe
    float random = ((float(rand()) / float(RAND_MAX)) * (3 - 4)) + 4;

	// mofification de orientation
	pthread_mutex_lock(&globalmutex.mtx_orientation);
    orientation = ctrl_orientation_buffer + random;
	pthread_mutex_unlock(&globalmutex.mtx_orientation);
	
 
}

void Navigation::compute_vitesse ()
{	
	// ajuste la vitesse à la consigne 

	// récuperrer la consigne de vitesse
	pthread_mutex_lock(&globalmutex.mtx_ctrl_vitesse);
	float ctrl_vitesse_buffer = ctrl_vitesse;
	pthread_mutex_unlock(&globalmutex.mtx_ctrl_vitesse);

    // Faire passer la vitesse à la consigne de manière douce (50 pas)
	pthread_mutex_lock(&globalmutex.mtx_vitesse);
    float pas  = (ctrl_vitesse_buffer - vitesse)/ 50;
	while ( vitesse != ctrl_vitesse_buffer)
	{
		vitesse += pas;
	}
	pthread_mutex_lock(&globalmutex.mtx_vitesse);
}

void Navigation::compute_batterie()
{	
	//lire la vitesse
	pthread_mutex_lock(&globalmutex.mtx_vitesse);
	float vitesse_buffer = vitesse;
	pthread_mutex_unlock(&globalmutex.mtx_vitesse);

	// calcule le niveau de batterie dans 20 m
	// (en supposant que la voiture consomme 0.1% de batterie par km
	pthread_mutex_lock(&globalmutex.mtx_lvl_batterie);
	lvl_batterie =  lvl_batterie- (lvl_batterie * (0.1 * vitesse_buffer / 3.6)*100);
	pthread_mutex_unlock(&globalmutex.mtx_lvl_batterie);
	if (lvl_batterie <= 10)
	{
		pthread_mutex_lock(&globalmutex.mtx_alarm10);
		alarm10 = true;
		pthread_mutex_unlock(&globalmutex.mtx_alarm10);
	}
	
}

bool Navigation::read_alarm10()
{
	// Permet de lire ctrl_charge avec le mutex
	pthread_mutex_lock(&globalmutex.mtx_alarm10);
	bool buffer = alarm10;
	pthread_mutex_unlock(&globalmutex.mtx_alarm10);
	return buffer;
	
}

bool Navigation::read_alarm80()
{
	// Permet de lire ctrl_charge avec le mutex
	pthread_mutex_lock(&globalmutex.mtx_alarm80);
	bool buffer = alarm80;
	pthread_mutex_unlock(&globalmutex.mtx_alarm80);
	return buffer;
	
}

bool Navigation::read_ctrl_charge()
{
	// Permet de lire ctrl_charge avec le mutex
	pthread_mutex_lock(&globalmutex.mtx_ctrl_charge);
	bool buffer = ctrl_charge ;
	pthread_mutex_unlock(&globalmutex.mtx_ctrl_charge);
	return buffer;
	
}

void Navigation::recharge_batterie ()
{
	pthread_mutex_lock(&globalmutex.mtx_lvl_batterie);
	while(lvl_batterie<80)
	{		
		lvl_batterie += 3;
	}
	pthread_mutex_unlock(&globalmutex.mtx_lvl_batterie);
	// avertir que la batterie est pleine
	pthread_mutex_lock(&globalmutex.mtx_alarm80);
	alarm80 = true;
	pthread_mutex_unlock(&globalmutex.mtx_alarm80);
}




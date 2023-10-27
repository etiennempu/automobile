#include "Navigation.h"



void Navigation::compute_orientation()
{
	
    //  symbolise le fait que la voiture dévie naturellement de son axe
    float random = ((float(rand()) / float(RAND_MAX)) * (3 - 4)) + 4;
	pthread_mutex_lock(orientation);
    orientation += random;
	pthread_mutex_unlock(orientation);
	
 
}

void Navigation::compute_vitesse ()
{	
	// ajuste la vitesse à la consigne 

	// récuperrer la consigne de vitesse
	pthread_mutex_lock(ctrl_vitesse);
	float ctrl_vitesse_buffer = ctrl_vitesse;
	pthread_mutex_unlock(ctrl_vitesse);

    // Faire passer la vitesse à la consigne de manière douce (50 pas)
	pthread_mutex_lock(vitesse);
    float pas  = (ctrl_vitesse_buffer - vitesse)/ 50;
	while ( vitesse != ctrl_vitesse_buffer)
	{
		vitesse += pas;
	}
	pthread_mutex_lock(vitesse);
}

void Navigation::compute_batterie()
{	
	//lire la vitesse
	pthread_mutex_lock(vitesse);
	float vitesse_buffer = vitesse;
	pthread_mutex_unlock(vitesse);

	// calcule le niveau de batterie dans 20 m
	// (en supposant que la voiture consomme 0.1% de batterie par km
	pthread_mutex_lock(lvl_batterie);
	lvl_batterie =  lvl_batterie- (lvl_batterie * (0.1 * vitesse_buffer / 3.6)*100);
	pthread_mutex_unlock(lvl_batterie);
	
}

bool Navigation::read_ctrl_charge()
{
	// Permet de lire avec le mutex
	pthread_mutex_lock();
	bool buffer = ctrl_charge ;
	pthread_mutex_unlock();
	return buffer;
	
}

void Navigation::recharge_batterie ()
{
	pthread_mutex_lock(lvl_batterie);
	while(lvl_batterie<80)
	{		
		lvl_batterie += 3;
	}
	pthread_mutex_unlock(lvl_batterie);

	pthread_join(alarm_80, NULL); a créer
	
}




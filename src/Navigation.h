#ifndef __NAVIGATION_H_
#define __NAVIGATION_H_

/**********************************************
 * 
 * Permet de modifier legerement l'orientation, 
 * 
 * ********************************************/

#include <cstdint>   /* Generic types */
#include <pthread.h> /* pthread_mutex_t, pthread_cond_t */
#include <iostream>
#include "genMap.h"



class Navigation
{
	public:
		Navigation();
		~Navigation();

		float get_vitesse();
		float get_orientation();
		void set_vitesse(float vitesse);
		void set_orientation(float orientation);
		void navigation();


	private:
		
		float target_orientation;
		float nav_orientation;
		float target_vitesse;
		float nav_vitesse; 
		float ecart_orientation();
		float compute_vitesse ();

		void ecart_orientation();
		void compute_vitesse ();
		

};

#endif

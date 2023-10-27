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
#include "sysContinu.h"



class Navigation
{
	public:
		/* les ancirnc
		
		float lvl_batterie;
		float target_orientation;
		float nav_orientation;
		float target_vitesse;
		float nav_vitesse; 
		*/
		

		// input
		float pos;
		float vitesse;
		float orientation;
		float lvl_batterie;
		// output
		float ctrl_charge;
		float ctrl_vitesse;
		float ctrl_orientation;
		
		void orientation();
		void vitesse ();
		void compute_batterie();
		void recharge_batterie ();


};

#endif

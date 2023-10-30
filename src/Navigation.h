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
		/* les anciens
		float lvl_batterie;
		float target_orientation;
		float nav_orientation;
		float target_vitesse;
		float nav_vitesse; 
		*/
		
		// input
		coord_t pos;
		float vitesse;
		float orientation;
		float lvl_batterie;
		// output
		bool ctrl_charge;
		float ctrl_vitesse;
		float ctrl_orientation;
		// alarm
		bool alarm10;
		bool alarm80;
		
		void compute_orientation();
		void compute_vitesse ();
		void compute_batterie();
		void recharge_batterie ();

		// permet de lire
		bool read_ctrl_charge();
		bool read_alarm10();
		bool read_alarm80();
};

#endif

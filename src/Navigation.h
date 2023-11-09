#ifndef __NAVIGATION_H_
#define __NAVIGATION_H_

/**********************************************
 * 
 * Permet de modifier legerement l'orientation, 
 * 
 * ********************************************/

 /* Generic types */
/* pthread_mutex_t, pthread_cond_t */


#include <iostream>
#include <cmath>
#include <random>
#include <cstring>
#include <time.h>
#include "genMap.h"

class Navigation
{
	public:

		

		coord_t pos;
		float vitesse;
		float orientation;
		float lvl_batterie;

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
		void position ();

		// permet de lire
		bool read_ctrl_charge();
		bool read_alarm10();
		bool read_alarm80();
};

#endif

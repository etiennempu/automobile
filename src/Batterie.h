#ifndef __BATTERIE_H_
#define __BATTERIE_H_

/**********************************************
 * 
 * Permet de recharger la batterie, consommer de la batterie
 * 
 * ********************************************/

#include <cstdint>   /* Generic types */
#include <pthread.h> /* pthread_mutex_t, pthread_cond_t */
#include <iostream>

class Batterie
{
	public: 
		Batterie();
		~Batterie();

		float get_batterie()
		{
			return lvl_batterie;
		}
		
	private:		

		float lvl_batterie = 60;

		float compute_batterie();
		void recharge_batterie ();		

};

#endif

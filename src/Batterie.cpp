#include "Batterie.h"
#include "Navigation.h"

void Batterie::compute_batterie()
{
	// calcule le niveau de batterie dans 20 m
	// (en supposant que la voiture consomme 0.1% de batterie par km
	float next_batterie;
	float vitesse =  nav.get_vitesse();
	lvl_batterie =  lvl_batterie- (lvl_batterie * (0.1 * vitesse / 3.6)*100);
	
}

void Batterie::recharge_batterie (float* batterie)
{
	while(lvl_batterie<80){
		
		lvl_batterie += 3;

	}
	
}

float Batterie::get_batterie()
{
	return lvl_batterie;
}





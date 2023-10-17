#include "Batterie.h"
#include "Navigation.h"

float compute_batterie()
{
	// calcule le niveau de batterie dans 20 m
	// (en supposant que la voiture consomme 0.1% de batterie par km
	float next_batterie;
	Navigation nav;
	float vitesse =  nav.get_vitesse();
	next_batterie =  Batterie::lvl_batterie- (Batterie::lvl_batterie * (0.1 * vitesse / 3.6)*100);
	return next_batterie;
}

void recharge_batterie (float* batterie)
{
	// rechargement de la batterie en 
	do
	{
		*Batterie::lvl_batterie += 3;
		// pour que ça soit pas instantané 
	} while (*Batterie::lvl_batterie != 80);	
}



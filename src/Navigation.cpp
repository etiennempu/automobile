#include "Navigation.h"



void Navigation::orientation()
{
	
    //  symbolise le fait que la voiture dévie naturellement de son axe
    float random = ((float(rand()) / float(RAND_MAX)) * (3 - 4)) + 4;
    nav_orientation += random;
	
 
}

void Navigation::vitesse ()
{
    // Faire passer la vitesse à la consigne de manière douce (50 pas)
    //float pas  = (target_vitesse - vitesse)/ 50;
	//vitesse += pas;
		// pour que ça soit pas instantané 
}

void Navigation::compute_batterie()
{
	// calcule le niveau de batterie dans 20 m
	// (en supposant que la voiture consomme 0.1% de batterie par km
	lvl_batterie =  lvl_batterie- (lvl_batterie * (0.1 * nav_vitesse / 3.6)*100);
	
}

void Navigation::recharge_batterie ()
{
	while(lvl_batterie<80){
		
		lvl_batterie += 3;

	}
	
}




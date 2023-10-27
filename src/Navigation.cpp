#include "Navigation.h"


float Navigation::get_vitesse()
{
	return nav_vitesse;
}

float Navigation::get_orientation()
{
	return nav_orientation;
}

void Navigation::set_vitesse(float vitesse)
{
	target_vitesse=vitesse;
}
void Navigation::set_orientation(float orientation)
{
	target_orientation=orientation;
}

void Navigation::orientation()
{
	
    //  symbolise le fait que la voiture dévie naturellement de son axe
    float random = ((float(rand()) / float(RAND_MAX)) * (3 - 4)) + 4;
    nav_orientation += random;
	
 
}

void Navigation::vitesse ()
{
    // Faire passer la vitesse à la consigne de manière douce (50 pas)
    float pas  = (target_vitesse - vitesse)/ 50;
	nav_vitesse += pas;
		// pour que ça soit pas instantané 
}

void Navigation::compute_batterie()
{
	// calcule le niveau de batterie dans 20 m
	// (en supposant que la voiture consomme 0.1% de batterie par km
	float next_batterie;
	float vitesse =  nav.get_vitesse();
	lvl_batterie =  lvl_batterie- (lvl_batterie * (0.1 * vitesse / 3.6)*100);
	
}

void Navigation::recharge_batterie (float* batterie)
{
	while(lvl_batterie<80){
		
		lvl_batterie += 3;

	}
	
}

float Navigation::get_batterie()
{
	return lvl_batterie;
}

void* Navigation::systemeContinu(void* args)
{

	orientation();
	vitesse();

}


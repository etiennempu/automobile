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

void Navigation::ecart_orientation()
{
    //  symbolise le fait que la voiture dévie naturellement de son axe

    float new_orientation;
    float random = ((float(rand()) / float(RAND_MAX)) * (3 - 4)) + 4;
    nav_orientation += random;
 
}

void Navigation::compute_vitesse ()
{
    // Faire passer la vitesse à la consigne de manière douce (50 pas)
    float pas  = (consigne_vitesse - vitesse)/ 50;
		vitesse += pas;
		// pour que ça soit pas instantané 
}

void Navigation::navigation()
{
	//
}
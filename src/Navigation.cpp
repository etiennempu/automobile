#include "Navigation.h"

float ecart_orientation(float orientation)
{
    //  symbolise le fait que la voiture dévie naturellement de son axe

    float new_orientation;
    float random = ((float(rand()) / float(RAND_MAX)) * (3 - 4)) + 4;
    new_orientation = orientation + random;
    return new_orientation;
}

float compute_vitesse (float vitesse, float consigne_vitesse)
{
    // Faire passer la vitesse à la consigne de manière douce (50 pas)
    float pas  = (consigne_vitesse - vitesse)/ 50;
    do
	{
		vitesse += pas;
		// pour que ça soit pas instantané 
	} while (vitesse != consigne_vitesse);
}
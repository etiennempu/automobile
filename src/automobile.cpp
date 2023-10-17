#include "automobile.h"
using namespace std;



//System Triggers
#define TRIGGER_ALARM_10 1
#define TRIGGER_ALARM_80 2

// System Inputs
#define LEVEL_BATTERIE 1
#define ANALYSE_TERMINEE 2
#define POS_X 3
#define POS_Y 4
#define ORIENTATION 5
#define VITESSE 6

// System Outputs
#define CTRL_CHARGE 1
#define CTRL_ANALYSE_PH 2
#define CTRL_ORIENTATION 3
#define CTRL_VITESSE 4

// System tasks periods (seconds)
#define CTRL_TASK_PERIOD (0.1)
#define CTRL_PHOTO_PERIOD (0.05)
#define CTRL_PRINT_PERIOD (1)


// Data structure used for the task data
struct TaskData {
    coord_t Position;
    coord_t OldPosition;
    float Vitesse;
    float Orientation;
    coord_t Destination;
    bool EtatBatterie;
    float LvlBatterie;
    coord_t LastPhotos;   //position de la derniere photo
    bool EtatDestination;
    coord_t *StationPossibles = new coord_t;
    bool AnalyseTerminee;
    PathMap My_PathMap;
    coord_t FinalDest;
    int final_dest;

    float Next_vitesse;
    float Next_orientation;
    coord_t Next_dest;  //coordonnees de la nouvelle destination
};

float compute_orientation(coord_t Position, coord_t Next_Dest) {
    float delta_x = Next_Dest.x - Position.x;
    float delta_y = Next_Dest.y - Position.y;
    float radians = 0;
     // Calcul de l'angle en radians entre l'axe x positif et le vecteur (delta_x, delta_y)
    if((delta_x < 0) && (delta_y < 0))
    {

        radians = atan2(abs(delta_x),abs(delta_y))+M_PI;
    }
    else if (delta_x < 0 && delta_y > 0)
    {
        radians = atan2(abs(delta_y), abs(delta_x))+(1.5*M_PI);
    }
    else if (delta_y < 0 && delta_x>0 )
    {
        radians = atan2(abs(delta_y), abs(delta_x))+(0.5*M_PI);
    }
    else
    {
        radians = atan2(delta_x, delta_y);
    }

    // Conversion de radians en degrés
    float degrees = radians * (180.0 / M_PI);

    return degrees;
}

float compute_distance(coord_t Position, coord_t Next_dest)
    /*
    * Role : calcule la distance entre deux points
    * Temps : 10 µs
    */
{
    float dist;
    dist = sqrt((Next_dest.x - Position.x)*(Next_dest.x - Position.x) + (Next_dest.y - Position.y)*(Next_dest.y - Position.y) );
    return dist;
}


void* Take_photo(void* data)
{
    /*
    * Role : prendre des photos
    * Note : cette tache doit etre reveillee suffisament regulierement pour ne pas rater le coche
    */
    TaskData *d = static_cast<TaskData*>(data);

        // Prendre une photo
            // cout << "\n Distance depuis la derniere photo : %f" << endl;, d->LastPhotos);
            if (compute_distance(d->LastPhotos,d->Position) >= 18)
            {
                d->My_PathMap.savePhoto(d->Position);
                d->LastPhotos = d->Position;
                ttAnalogOut(CTRL_ANALYSE_PH, 1);

                // Analyse terminee --> Ne plus prendre photo
				if(d->AnalyseTerminee == 1)
				{
				  ttAnalogOut(CTRL_ANALYSE_PH, 0);
				}

            }
}


void* Navigation(void* data){
    /*
    * Role : recuperer en permanence les donnees, les print et mettre a jours les consignes
    */
    TaskData *d = static_cast<TaskData*>(data);

	// recuperation des donnees
	d->Vitesse = ttAnalogIn(VITESSE);
	coord_t Position;
	Position.x = ttAnalogIn(POS_X);
	Position.y = ttAnalogIn(POS_Y);
	d->Position = Position;
	d->Orientation = ttAnalogIn(ORIENTATION);
	d->LvlBatterie = ttAnalogIn(LEVEL_BATTERIE);
	d->AnalyseTerminee = ttAnalogIn(ANALYSE_TERMINEE);

	pthread_join(FSM_Tid, NULL);

	// send ouputs to operative system
	ttAnalogOut(CTRL_VITESSE, d->Next_vitesse);
	ttAnalogOut(CTRL_ORIENTATION, d->Next_orientation);
}

void* FSM(void* data){
    /*
    * Role : Machine a etat du systeme
    */
    TaskData *d = static_cast<TaskData*>(data);

	// Calcul de la nouvelle orientation a prendre
	d->Next_orientation = compute_orientation(d->Position, d->Next_dest);

	if(d->final_dest>0)
	{
		if(d->final_dest==600)
		{
			d->Next_vitesse=80;
			d->final_dest=0;
		}
		else
		{
			d->final_dest++;
		}
	}

	if(d->Next_vitesse==0)
	{
	}

	// Arrivee proche
	if ( compute_distance(d->Position, d->Next_dest) <= 100)
	{
		d->Next_vitesse = 50;

	}

	// Cas d'arret
	if( compute_distance(d->Position, d->Next_dest) < 10)
	{
		// Arrivee finale
		if ((d->FinalDest.x == d->Next_dest.x) && (d->FinalDest.y == d->Next_dest.y))
		{
			d->Next_vitesse = 0;
			cout << endl << "arrivee dest finale : Wait 60 s" << endl;
			d->My_PathMap.genDest(d->Position, d->FinalDest);
			d->My_PathMap.genWp(d->Position, d->FinalDest, d->Next_dest);
			d->final_dest++;
		}

		// Recharge
		else if (d->EtatBatterie)
		{
			d->Next_vitesse = 0;
			cout << endl << "arrivee a une borne" << endl;
			ttAnalogOut(CTRL_CHARGE, true);
			d->EtatBatterie = false ;

		}
		// Arrivee wp
		else
		{
			d->Next_vitesse = 80;
			cout << endl << "Arrivee a un wp" << endl;
			d->My_PathMap.genWp(d->Position, d->FinalDest, d->Next_dest);
		}
	}
}

void* print_state(void* data){
    /*
    * Role : monitorer les valeurs
    */
    TaskData *d = static_cast<TaskData*>(data);

	cout << endl << "Etat voiture :" << endl;
	cout << "--> Vitesse     :" << d->Vitesse << "km/h" << endl;
	cout << "--> Position    : (" << d->Position.x << ", " << d->Position.y << ")" << endl;
	cout << "--> Orientation : " << d->Orientation << " degre" << endl;
	cout << "--> Batterie    : " << d->LvlBatterie << endl;
	cout << "--> NextDest    : (" << d->Next_dest.x << ", " << d->Next_dest.y << ") %" << endl;

}


void* alarm_10(void* data) {
    /*
    * Role : s'execute en cas d'alarme 10% de batterie
    */
    TaskData *d = static_cast<TaskData*>(data);

	cout << "Alarme 10%" << endl;
	// mettre a jours les nouvelles valeurs
	d->Next_vitesse = 50;
	d->My_PathMap.getClosestStation(d->Position, d->Next_dest);
	d->EtatBatterie = true ;
	pthread_join(FSM_Tid, NULL);

}

void* alarm_80(void* data) {
    /*
    * Role : s'execute en cas d'alarme 80% de batterie
    */
    TaskData *d = static_cast<TaskData*>(data);
	cout << "Alarme 80%" << endl;
	ttAnalogOut(CTRL_CHARGE, false);
	d->Next_vitesse = 80;
	d->My_PathMap.genWp(d->Position, d->FinalDest, d->Next_dest);
	pthread_join(FSM_Tid, NULL);
}

int main() {
	cout << "Starting simulation " << endl;


	//----------------------------- initialisation des mutex communs avec la partie continue -------------------------------
	// input
	pthread_mutex_init(&mtx_pos_x, NULL);
	pthread_mutex_init(&mtx_pos_y, NULL);
	pthread_mutex_init(&mtx_vitesse, NULL);
	pthread_mutex_init(&mtx_orientation, NULL);
	pthread_mutex_init(&mtx_lvl_batterie, NULL);
	// output
	pthread_mutex_init(&mtx_ctrl_charge, NULL);
	pthread_mutex_init(&mtx_ctrl_vitesse, NULL);
	pthread_mutex_init(&mtx_ctrl_orientation, NULL);
	// ------------------------------------------------------------------------------------------------------------------
	continious_t* My_cont;

	TaskData *data = new TaskData;
	coord_t Position;

	pthread_mutex_lock(POS_X);
	Position.x = POS_X;
	pthread_mutex_unlock(POS_X);
	Position.y = ttAnalogIn(POS_Y);
	data->Position = Position;
	data->My_PathMap.genDest(data->Position, data->FinalDest); // donner la destination finale initiale
	data->My_PathMap.genWp(data->Position, data->FinalDest, data->Next_dest);
	data->Next_orientation = compute_orientation(data->Position, data->Next_dest);
	// commencer a se mettre en routes
	data->Next_vitesse = 80;
	data->LastPhotos = data->Position;
	data->EtatBatterie = false ;
	data->final_dest=0;

	// ------------------------------- creation des threads -----------------------------------
	pthread_t alarm_low_Tid, alarm_high_Tid, FSM_Tid, nav_Tid, print_Tid, continu_Tid;

	if(0 != pthread_create(&continu_Tid, NULL, run_continu, (void*)My_cont))
	{
		/* Print error */
		printf("Could not create thread: %d\n", errno);
		return EXIT_FAILURE;
	}

	if(0 != pthread_create(&FSM_Tid, NULL, FSM, &data))
	{
		/* Print error */
		printf("Could not create thread: %d\n", errno);
		return EXIT_FAILURE;
	}
	if(0 != pthread_create(&print_Tid, NULL, print_state, &data))
	{
		/* Print error */
		printf("Could not create thread: %d\n", errno);
		return EXIT_FAILURE;
	}

	pthread_join(alarm_low_Tid, NULL);
	pthread_join(alarm_high_Tid, NULL);
	pthread_join(nav_Tid, NULL);
	pthread_join(print_Tid, NULL);


	return 0;
}

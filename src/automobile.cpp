#include <cstdint>   /* Generic types */
#include <pthread.h> /* pthread_mutex_t, pthread_cond_t */
#include <iostream>
#include "genMap.h"
#include <cmath>    // Pour avoir des fonctions mathematiques
#include "sysContinu.h"
#include "timer.h"
#include "Navigation.h"
using namespace std;


// System tasks periods (ms)
#define CTRL_TASK_PERIOD (100)
#define CTRL_PHOTO_PERIOD (50)
#define CTRL_PRINT_PERIOD (1000)
#define CTRL_CONTINUS_PERIOD (10)

//durée de la simulation en seconde
#define MAX_EXECUTION_TIME 1000

#define 

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
	Navigation myNav;
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
	void **params = (void **)data;

    TaskData *d = static_cast<TaskData*>(params[0]);

	struct timespec tp;
	sem_t*          sync_sem;
	uint32_t        task_id;
	uint32_t        starttime;
	uint32_t        elapsed_time;

	/* Get the arguments */
	sync_sem  = ((thread_args_t*)params[1])->semaphore;
	task_id   = ((thread_args_t*)params[1])->id;
	starttime = ((thread_args_t*)params[1])->starttime;
	uint32_t max_execution_time = MAX_EXECUTION_TIME;
	uint32_t endtime = starttime + max_execution_time;

	int run=1;
	/* Routine loop */
	while(run==1) {
		/* Wait for the pulse handler to release the semaphore */
		if(0 == sem_wait(sync_sem)) {
			/* Get the current time */
			if(0 == clock_gettime(CLOCK_REALTIME, &tp)) {
				elapsed_time = tp.tv_sec - starttime;
				
			    // Prendre une photo
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

				if (tp.tv_sec >= endtime) {
                    printf("Task %d has completed execution.\n", task_id);
                    run=0;; // Exit the loop
                }

			}
			else {
				/* Print error */
				printf("Task %d could not get time: %d\n", task_id, errno);
			}
		}
		else {
			printf("Task %d could not wait semaphore: %d\n", task_id, errno);
		}
	}


}


void Navigation(TaskData* data){
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
   	void **params = (void **)data;

    TaskData *d = static_cast<TaskData*>(params[0]);

	struct timespec tp;
	sem_t*          sync_sem;
	uint32_t        task_id;
	uint32_t        starttime;
	uint32_t        elapsed_time;

	/* Get the arguments */
	sync_sem  = ((thread_args_t*)params[1])->semaphore;
	task_id   = ((thread_args_t*)params[1])->id;
	starttime = ((thread_args_t*)params[1])->starttime;
	uint32_t max_execution_time = MAX_EXECUTION_TIME;
	uint32_t endtime = starttime + max_execution_time;
    TaskData *d = static_cast<TaskData*>(data);

	int run=1:
	/* Routine loop */
	while(run==1) {

		/* Wait for the pulse handler to release the semaphore */
		if(0 == sem_wait(sync_sem)) {
			/* Get the current time */
			if(0 == clock_gettime(CLOCK_REALTIME, &tp)) {
				elapsed_time = tp.tv_sec - starttime;

			Navigation(params[0]);
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
			

            /*else if(d->Next_vitesse==0)
            {

            }*/
            else
			{
				// Arrivee proche
				if ( compute_distance(d->Position, d->Next_dest) <= 100)
            	{                
                	d->Next_vitesse = 50;
               
            	}

				if( compute_distance(d->Position, d->Next_dest) < 10)
            	{
                // Arrivee finale
                	if ((d->FinalDest.x == d->Next_dest.x) && (d->FinalDest.y == d->Next_dest.y))
					{
						d->Next_vitesse = 0;
						mexPrintf("\n arrivee dest finale : Wait 60 s\n");
						d->My_PathMap.genDest(d->Position, d->FinalDest);
						d->My_PathMap.genWp(d->Position, d->FinalDest, d->Next_dest);
						d->final_dest++;
					}
                
				// Recharge
					else if (d->EtatBatterie)
					{   
						d->Next_vitesse = 0;             
						mexPrintf("\nArrivee a une borne\n");
						ttAnalogOut(CTRL_CHARGE, true);
						d->EtatBatterie = false ;
	
					}
					// Arrivee wp
					else 
					{                
						d->Next_vitesse = 80;
						mexPrintf("\nArrivee a un wp\n");
						d->My_PathMap.genWp(d->Position, d->FinalDest, d->Next_dest);   
					} 

            	}
			}

				if (tp.tv_sec >= endtime) {
                    printf("Task %d has completed execution.\n", task_id);
                    run=0; // Exit the loop
                }

			}
			else {
				/* Print error */
				printf("Task %d could not get time: %d\n", task_id, errno);
			}
		}
		else {
			printf("Task %d could not wait semaphore: %d\n", task_id, errno);
		}
	}

	
}

void* print_state(void* data){
    /*
    * Role : monitorer les valeurs
    */
   	void **params = (void **)data;

    TaskData *d = static_cast<TaskData*>(params[0]);

	struct timespec tp;
	sem_t*          sync_sem;
	uint32_t        task_id;
	uint32_t        starttime;
	uint32_t        elapsed_time;

	/* Get the arguments */
	sync_sem  = ((thread_args_t*)params[1])->semaphore;
	task_id   = ((thread_args_t*)params[1])->id;
	starttime = ((thread_args_t*)params[1])->starttime;
	uint32_t max_execution_time = MAX_EXECUTION_TIME;
	uint32_t endtime = starttime + max_execution_time;

	int run=1;
	/* Routine loop */
	while(run==1) {
		/* Wait for the pulse handler to release the semaphore */
		if(0 == sem_wait(sync_sem)) {
			/* Get the current time */
			if(0 == clock_gettime(CLOCK_REALTIME, &tp)) {
				elapsed_time = tp.tv_sec - starttime;
				
				cout << endl << "Etat voiture :" << endl;
				cout << "--> Vitesse     :" << d->Vitesse << "km/h" << endl;
				cout << "--> Position    : (" << d->Position.x << ", " << d->Position.y << ")" << endl;
				cout << "--> Orientation : " << d->Orientation << " degre" << endl;
				cout << "--> Batterie    : " << d->LvlBatterie << endl;
				cout << "--> NextDest    : (" << d->Next_dest.x << ", " << d->Next_dest.y << ") %" << endl;

				if (tp.tv_sec >= endtime) {
                    printf("Task %d has completed execution.\n", task_id);
                    run=0;; // Exit the loop
                }

			}
			else {
				/* Print error */
				printf("Task %d could not get time: %d\n", task_id, errno);
			}
		}
		else {
			printf("Task %d could not wait semaphore: %d\n", task_id, errno);
		}
	}

	
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
int main(void) {

	cout << "Starting simulation " << endl;

		
	continious_t globalmutex;
	initContinious_t(&globalmutex);

	srand (time(NULL));
    
    TaskData *data = new TaskData;  
    const char* image_generee; 

	data->myNav= new Navigation;

	coord_t Position;

	data->myNav.set;
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

	/////timer
	struct timespec tp;

	/* Semaphores for pulse synchronization */
	sem_t task_control_sync;
	sem_t task_photo_sync;
	sem_t task_print_sync;
	sem_t task_continus_sync;


	/* PThread structures */
	pthread_t task_control;
	pthread_t task_control_pulse_handler;
	pthread_t task_photo;
	pthread_t task_photo_pulse_handler;
	pthread_t task_print;
	pthread_t task_print_pulse_handler;
	pthread_t task_continus;
	pthread_t task_continus_pulse_handler;

	/* Timers event structures */
	struct sigevent   task_control_event;
	struct itimerspec task_control_itime;
	timer_t           task_control_timer;
	struct sigevent   task_photo_event;
	struct itimerspec task_photo_itime;
	timer_t           task_photo_timer;
	struct sigevent   task_print_event;
	struct itimerspec task_print_itime;
	timer_t           task_print_timer;
	struct sigevent   task_continus_event;
	struct itimerspec task_continus_itime;
	timer_t           task_continus_timer;

	/* Tasks arguments */
	thread_args_t task_control_args;
	thread_args_t task_photo_args;
	thread_args_t task_print_args;
	thread_args_t task_continus_args;




	/* Get the start time */
	if(0 != clock_gettime(CLOCK_REALTIME, &tp)) {
		/* Print error */
		printf("Could not get start time: %d\n", errno);
		return EXIT_FAILURE;
	}

	/* Initialize the semaphore */
	if(0 != sem_init(&task_control_sync, 0, 0)) {
		/* Print error */
		printf("Could not get init semaphore: %d\n", errno);
		return EXIT_FAILURE;
	}
	if(0 != sem_init(&task_photo_sync, 0, 0)) {
		/* Print error */
		printf("Could not get init semaphore: %d\n", errno);
		return EXIT_FAILURE;
	}
		if(0 != sem_init(&task_print_sync, 0, 0)) {
		/* Print error */
		printf("Could not get init semaphore: %d\n", errno);
		return EXIT_FAILURE;
	}

	/* Initialize the tasks arguments */
	task_control_args.id   = 0;
	task_control_args.semaphore = &task_control_sync;
	task_control_args.starttime = tp.tv_sec;
	task_control_args.chid      = ChannelCreate(0);
	if(-1 == task_control_args.chid) {
		/* Print error */
		printf("Could not create channel: %d\n", errno);
		return EXIT_FAILURE;
	}

	task_photo_args.id   = 1;
	task_photo_args.semaphore = &task_photo_sync;
	task_photo_args.starttime = tp.tv_sec;
	task_photo_args.chid      = ChannelCreate(0);
	if(-1 == task_photo_args.chid) {
		/* Print error */
		printf("Could not create channel: %d\n", errno);
		return EXIT_FAILURE;
	}

	task_print_args.id   = 2;
	task_print_args.semaphore = &task_print_args;
	task_print_args.starttime = tp.tv_sec;
	task_print_args.chid      = ChannelCreate(0);
	if(-1 == task_print_args.chid) {
		/* Print error */
		printf("Could not create channel: %d\n", errno);
		return EXIT_FAILURE;
	}
	task_continus_args.id   = 3;
	task_continus_args.semaphore = &task_continus_args;
	task_continus_args.starttime = tp.tv_sec;
	task_continus_args.chid      = ChannelCreate(0);

	if(-1 == task_continus_args.chid) {
		/* Print error */
		printf("Could not create channel: %d\n", errno);
		return EXIT_FAILURE;
	}

	void *params_control[2];
	params_control[0] = data;
	params_control[1] = &task_control_args;

	/* Create the different tasks and their associated pulse handlers */
	if(0 != pthread_create(&task_control, NULL, FSM, params_control)) {
		/* Print error */
		printf("Could not create thread: %d\n", errno);
		return EXIT_FAILURE;
	}
	if(0 != pthread_create(&task_control_pulse_handler, NULL,
			               task_pulse_handler, &task_control_args)) {
		/* Print error */
		printf("Could not create thread: %d\n", errno);
		return EXIT_FAILURE;
	}

	void *params_photo[2];
	params_photo[0] = data;
	params_photo[1] = &task_control_args;

	if(0 != pthread_create(&task_photo, NULL, Take_photo, params_control)) {
		/* Print error */
		printf("Could not create thread: %d\n", errno);
		return EXIT_FAILURE;
	}
	if(0 != pthread_create(&task_photo_pulse_handler, NULL, 
							task_pulse_handler, &task_photo_args)) {
		/* Print error */
		printf("Could not create thread: %d\n", errno);
		return EXIT_FAILURE;
	}

	void *params_print[2];
	params_print[0] = data;
	params_print[1] = &task_print_args;
	if(0 != pthread_create(&task_print, NULL, print_state, params_print)) {
		/* Print error */
		printf("Could not create thread: %d\n", errno);
		return EXIT_FAILURE;
	}
	if(0 != pthread_create(&task_print_pulse_handler, NULL,
			               task_pulse_handler, &task_print_args)) {
		/* Print error */
		printf("Could not create thread: %d\n", errno);
		return EXIT_FAILURE;
	}

	void *params_continus[2];
	params_continus[0] = data;
	params_continus[1] = &task_print_args;
	if(0 != pthread_create(&task_continus, NULL, print_continus, params_continus)) {
		/* Print error */
		printf("Could not create thread: %d\n", errno);
		return EXIT_FAILURE;
	}
	if(0 != pthread_create(&task_continus_pulse_handler, NULL,
			               task_pulse_handler, &task_continus_args)) {
		/* Print error */
		printf("Could not create thread: %d\n", errno);
		return EXIT_FAILURE;
	}

	/* Create timers */
	if(0 != init_timer(&task_control_event, &task_control_itime, &task_control_timer,
			           task_control_args.chid, CTRL_TASK_PERIOD)) {
		/* Print error */
		printf("Could not create timer: %d\n", errno);
		return EXIT_FAILURE;
	}
	if(0 != init_timer(&task_photo_event, &task_photo_itime, &task_photo_timer,
			           task_photo_args.chid, CTRL_PHOTO_PERIOD)) {
		/* Print error */
		printf("Could not create timer: %d\n", errno);
		return EXIT_FAILURE;
	}

	if(0 != init_timer(&task_print_event, &task_print_itime, &task_print_timer,
			           task_print_args.chid, CTRL_PRINT_PERIOD)) {
		/* Print error */
		printf("Could not create timer: %d\n", errno);
		return EXIT_FAILURE;
	}

	if(0 != init_timer(&task_continus_event, &task_continus_itime, &task_continus_timer,
			           task_print_args.chid, CTRL_CONTINUS_PERIOD)) {
		/* Print error */
		printf("Could not create timer: %d\n", errno);
		return EXIT_FAILURE;
	}

	/* Wait for the threads to finish */
	if(0 != pthread_join(task_control, NULL)) {
		/* Print error */
		printf("Could not wait for thread: %d\n", errno);
		return EXIT_FAILURE;
	}
	if(0 != pthread_join(task_control_pulse_handler, NULL)) {
		/* Print error */
		printf("Could not wait for thread: %d\n", errno);
		return EXIT_FAILURE;
	}
	if(0 != pthread_join(task_photo, NULL)) {
		/* Print error */
		printf("Could not wait for thread: %d\n", errno);
		return EXIT_FAILURE;
	}
	if(0 != pthread_join(task_photo_pulse_handler, NULL)) {
		/* Print error */
		printf("Could not wait for thread: %d\n", errno);
		return EXIT_FAILURE;
	}

	if(0 != pthread_join(task_print, NULL)) {
		/* Print error */
		printf("Could not wait for thread: %d\n", errno);
		return EXIT_FAILURE;
	}
	if(0 != pthread_join(task_print_pulse_handler, NULL)) {
		/* Print error */
		printf("Could not wait for thread: %d\n", errno);
		return EXIT_FAILURE;
	}

	if(0 != pthread_join(task_continus, NULL)) {
		/* Print error */
		printf("Could not wait for thread: %d\n", errno);
		return EXIT_FAILURE;
	}
	if(0 != pthread_join(task_continus_pulse_handler, NULL)) {
		/* Print error */
		printf("Could not wait for thread: %d\n", errno);
		return EXIT_FAILURE;
	}



	return EXIT_SUCCESS;
}
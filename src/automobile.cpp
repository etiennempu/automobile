  /* Generic types */ /* pthread_mutex_t, pthread_cond_t */

#include "genMap.h"
#include <cmath>    // Pour avoir des fonctions mathematiques
#include "sysContinu.h"
#include "timer.h"
#include "Navigation.h"
#include <semaphore.h>
using namespace std;


// System tasks periods (ms)
#define CTRL_TASK_PERIOD (100)
#define CTRL_PHOTO_PERIOD (50)
#define CTRL_PRINT_PERIOD (1000)
#define CTRL_CONTINUS_PERIOD (10)


// Data structure used for the task data
struct TaskData {
	coord_t Position;
    coord_t OldPosition;
    float Vitesse;
    float Orientation;
    coord_t Destination;
    bool EtatBatterie;
    bool Chargement;
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
	Navigation my_Nav;
	int sync_photo;
};
void* alarm_80(void* data);
void* alarm_10(void* data);
void* print_state(void* data);
void* FSM(void* data);
void Navigation2(TaskData* data);
void* Take_photo(void* data);
float compute_distance(coord_t Position, coord_t Next_dest);
float compute_orientation(coord_t Position, coord_t Next_Dest);
void* systemeContinu(void* data);
void* call_photo(void* data);

continious_t globalmutex;

int main(void) {

	cout << "Starting simulation " << endl;


	initContinious_t(&globalmutex);

	srand (time(NULL));

    TaskData *data = new TaskData;

	coord_t Position;
	Position.x=0;
	Position.y=0;

	data->LvlBatterie=0;
	data->Position = Position;
	data->My_PathMap.genDest(data->Position, data->FinalDest); // donner la destination finale initiale
	data->My_PathMap.genWp(data->Position, data->FinalDest, data->Next_dest);
	data->Next_orientation = compute_orientation(data->Position, data->Next_dest);
	// commencer a se mettre en routes
	data->Next_vitesse = 80;
	data->LastPhotos = Position;
	data->EtatBatterie = false ;
	data->Chargement = false;
	data->sync_photo = 0;


	data->my_Nav.alarm10 = false;
	data->my_Nav.alarm80 = false;

	data->my_Nav.ctrl_charge = 0;
	data->my_Nav.ctrl_orientation = data->Next_orientation;
	data->my_Nav.ctrl_vitesse = data->Next_vitesse;

	data->my_Nav.lvl_batterie = 60;

	data->my_Nav.pos = Position;

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
	pthread_t task_call_photo;


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


	/* Priorité */
    // Créez une structure sched_param pour spécifier la priorité
    struct sched_param continus_param;
    struct sched_param control_param;
    struct sched_param photo_param;
    struct sched_param print_param;
    struct sched_param call_photo_param;

    continus_param.sched_priority = 3; // Priorité élevée pour la tâche continus
    control_param.sched_priority = 5;  // Priorité moyenne pour la tâche de contrôle
    photo_param.sched_priority = 5;    // Priorité basse pour la tâche de photo
    print_param.sched_priority = 2;    // Priorité moyenne pour la tâche d'impression
    call_photo_param.sched_priority = 4;    //


	//les attributs de planification avec les priorités
    pthread_attr_t continus_attr;
	pthread_attr_t control_attr;
	pthread_attr_t photo_attr;
	pthread_attr_t print_attr;
	pthread_attr_t call_photo_attr;

	// Initialisation des attributs
	pthread_attr_init(&continus_attr);
	pthread_attr_init(&control_attr);
	pthread_attr_init(&photo_attr);
	pthread_attr_init(&print_attr);
	pthread_attr_init(&call_photo_attr);

	// les priorités pour les attributs de planification
	pthread_attr_setschedpolicy(&continus_attr, SCHED_RR);
	pthread_attr_setschedparam(&continus_attr, &continus_param);

	pthread_attr_setschedpolicy(&control_attr, SCHED_RR);
	pthread_attr_setschedparam(&control_attr, &control_param);

	pthread_attr_setschedpolicy(&photo_attr, SCHED_RR);
	pthread_attr_setschedparam(&photo_attr, &photo_param);

	pthread_attr_setschedpolicy(&print_attr, SCHED_RR);
	pthread_attr_setschedparam(&print_attr, &print_param);

	pthread_attr_setschedpolicy(&call_photo_attr, SCHED_RR);
	pthread_attr_setschedparam(&call_photo_attr, &call_photo_param);



	/* Tasks arguments */
	thread_args_t task_control_args;
	thread_args_t task_photo_args;
	thread_args_t task_print_args;
	thread_args_t task_continus_args;


	/* Get the start time */
	if(0 != clock_gettime(CLOCK_MONOTONIC, &tp)) {
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
		if(0 != sem_init(&task_continus_sync, 0, 0)) {
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
	task_print_args.semaphore = &task_print_sync;
	task_print_args.starttime = tp.tv_sec;
	task_print_args.chid      = ChannelCreate(0);
	if(-1 == task_print_args.chid) {
		/* Print error */
		printf("Could not create channel: %d\n", errno);
		return EXIT_FAILURE;
	}
	task_continus_args.id   = 3;
	task_continus_args.semaphore = &task_continus_sync;
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

	if(0 != pthread_create(&task_control, &control_attr, FSM , params_control)) {
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
	params_photo[1] = &task_photo_args;


	if(0 != pthread_create(&task_photo, &photo_attr, Take_photo, params_photo)) {
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
	if(0 != pthread_create(&task_print, &print_attr, print_state, params_print)) {
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
	params_continus[1] = &task_continus_args;
	if(0 != pthread_create(&task_continus, &continus_attr, systemeContinu, params_continus)) {
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


	if(0 != pthread_create(&task_call_photo, &call_photo_attr, call_photo, (void*)data)) {
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
			           task_continus_args.chid, CTRL_CONTINUS_PERIOD)) {
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

    pthread_attr_destroy(&continus_attr);
    pthread_attr_destroy(&control_attr);
    pthread_attr_destroy(&photo_attr);
    pthread_attr_destroy(&print_attr);

	cout << "Threads ended" << endl;

	data->My_PathMap.dumpImage("test.bmp"); // generer l'image finale

	delete data;
	cout << "Simulation ended" << endl;

	//return EXIT_SUCCESS;
}

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

void* call_photo(void* data){
    /*
    * Role : prendre les photos
    */
   	void **params = (void **)data;
    TaskData *d = static_cast<TaskData*>((TaskData*)params);

    while(d->sync_photo!=3)
    {
    	while(d->sync_photo!=1){}
        d->My_PathMap.takePhoto(d->LastPhotos);
        d->sync_photo=0;
    }
}

void* Take_photo(void* data)
{
	/*
    * Role : prendre des photos
    * Note : cette tache doit etre reveillee suffisament regulierement pour ne pas rater le coche
    */
	void **params = (void **)data;

    TaskData *d = static_cast<TaskData*>((TaskData*)params[0]);

	struct timespec tp;
	sem_t*          sync_sem;
	uint32_t        task_id;
	uint32_t        starttime;
	uint32_t        elapsed_time;

	/* Get the arguments */
	sync_sem  = ((thread_args_t*)params[1])->semaphore;
	task_id   = ((thread_args_t*)params[1])->id;
	starttime = ((thread_args_t*)params[1])->starttime;
	uint32_t max_execution_time = MAX_EXECUTION_TIME*1000;
	uint32_t endtime =  starttime + max_execution_time;
	int* sync_photo = ((int*)params[2]);

	/* Routine loop */
	while(((thread_args_t*)params[1])->run==1) {
		/* Wait for the pulse handler to release the semaphore */
		if(0 == sem_wait(sync_sem)) {
			/* Get the current time */
			if(0 == clock_gettime(CLOCK_MONOTONIC, &tp)) {
				elapsed_time = (tp.tv_sec - starttime) * 1000 + tp.tv_nsec / 1000000;

			    // Prendre une photo
				if (compute_distance(d->LastPhotos,d->Position) >= 18)
				{


					d->LastPhotos = d->Position;
					//cas où la photo est finie
					if(d->sync_photo==0)
					{
						d->sync_photo=1;
					}
					//sinon on skip cette photo;
				}

				if (elapsed_time >= endtime) {
                    printf("Task %d has completed execution.\n", task_id);
                    // Exit the loop
                    ((thread_args_t*)params[1])->run=0;
                    d->sync_photo=3;
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


void Navigation2(TaskData* data){
    /*
    * Role : recuperer en permanence les donnees, les print et mettre a jours les consignes
    */
    TaskData *d = static_cast<TaskData*>(data);

	// recuperation des donnees
	pthread_mutex_lock(&globalmutex.mtx_vitesse);
	d->Vitesse = d->my_Nav.vitesse;
	pthread_mutex_unlock(&globalmutex.mtx_vitesse);


	pthread_mutex_lock(&globalmutex.mtx_pos);
	d->Position.x = d->my_Nav.pos.x;
	d->Position.y = d->my_Nav.pos.y;
	pthread_mutex_unlock(&globalmutex.mtx_pos);

	pthread_mutex_lock(&globalmutex.mtx_orientation);
	d->Orientation = d->my_Nav.orientation;
	pthread_mutex_unlock(&globalmutex.mtx_orientation);

	pthread_mutex_lock(&globalmutex.mtx_lvl_batterie);
	d->LvlBatterie = d->my_Nav.lvl_batterie;
	pthread_mutex_unlock(&globalmutex.mtx_lvl_batterie);


	//d->AnalyseTerminee = ttAnalogIn(ANALYSE_TERMINEE);  Voir quoi faire avec ça


	// send ouputs to operative system
	pthread_mutex_lock(&globalmutex.mtx_ctrl_vitesse);
	d->my_Nav.ctrl_vitesse = d->Next_vitesse;
	pthread_mutex_unlock(&globalmutex.mtx_ctrl_vitesse);

	pthread_mutex_lock(&globalmutex.mtx_ctrl_orientation);
	d->my_Nav.ctrl_orientation = d->Next_orientation;
	pthread_mutex_unlock(&globalmutex.mtx_ctrl_orientation);
}


void* FSM(void* data){
    /*
    * Role : Machine a etat du systeme
    */
	void **params = (void **)data;

    TaskData *d = static_cast<TaskData*>((TaskData*)params[0]);

	struct timespec tp;
	sem_t*          sync_sem;
	uint32_t        task_id;
	uint32_t        starttime;
	uint32_t        elapsed_time;

	/* Get the arguments */
	sync_sem  = ((thread_args_t*)params[1])->semaphore;
	task_id   = ((thread_args_t*)params[1])->id;
	starttime = ((thread_args_t*)params[1])->starttime;
	uint32_t max_execution_time = MAX_EXECUTION_TIME*1000;
	uint32_t endtime =  starttime + max_execution_time;


	/* Routine loop */
	while(((thread_args_t*)params[1])->run==1) {
		/* Wait for the pulse handler to release the semaphore */
		if(0 == sem_wait(sync_sem)) {

			/* Get the current time */
			if(0 == clock_gettime(CLOCK_MONOTONIC, &tp)) {
				elapsed_time = (tp.tv_sec - starttime) * 1000 + tp.tv_nsec / 1000000;


				Navigation2(d);
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


				else if(d->Next_vitesse==0)
				{

				}
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
							cout << endl << "arrivee dest finale : Wait 60 s:" << endl;
							d->My_PathMap.genDest(d->Position, d->FinalDest);
							d->My_PathMap.genWp(d->Position, d->FinalDest, d->Next_dest);
							d->final_dest++;
						}

					// Recharge
						else if (d->EtatBatterie)
						{

							d->Next_vitesse = 0;
							cout << endl << "Arrivé a une borne :" << endl;
							pthread_mutex_lock(&globalmutex.mtx_ctrl_charge);
							d->my_Nav.ctrl_charge = 1;
							pthread_mutex_unlock(&globalmutex.mtx_ctrl_charge);

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

					if (elapsed_time >= endtime) {
						printf("Task %d has completed execution.\n", task_id);
						// Exit the loop
						((thread_args_t*)params[1])->run=0;
	
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

    TaskData *d = static_cast<TaskData*>((TaskData*)params[0]);

	struct timespec tp;
	sem_t*          sync_sem;
	uint32_t        task_id;
	uint32_t        starttime;
	uint32_t        elapsed_time;

	/* Get the arguments */
	sync_sem  = ((thread_args_t*)(params[1]))->semaphore;
	task_id   = ((thread_args_t*)(params[1]))->id;
	starttime = ((thread_args_t*)(params[1]))->starttime;
	uint32_t max_execution_time = MAX_EXECUTION_TIME*1000;
	uint32_t endtime = starttime + max_execution_time;


	/* Routine loop */
	while(((thread_args_t*)params[1])->run==1) {
		/* Wait for the pulse handler to release the semaphore */
		if(0 == sem_wait(sync_sem)) {
			/* Get the current time */
			if(0 == clock_gettime(CLOCK_MONOTONIC, &tp)) {
				elapsed_time = (tp.tv_sec - starttime) * 1000 + tp.tv_nsec / 1000000;


				cout << endl << "Etat voiture :" << endl;
				cout << "--> Vitesse     :" << d->Vitesse << "km/h" << endl;
				cout << "--> Position    : (" << d->Position.x << ", " << d->Position.y << ")" << endl;
				cout << "--> Orientation : " << d->Orientation << " degre" << endl;
				cout << "--> Batterie    : " << d->LvlBatterie << endl;
				cout << "--> NextDest    : (" << d->Next_dest.x << ", " << d->Next_dest.y << ")" << endl;

				if (elapsed_time >= endtime) {
                    printf("Task %d has completed execution.\n", task_id);
                    // Exit the loop
                    ((thread_args_t*)params[1])->run=0;

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

}

void* alarm_80(void* data) {
    /*
    * Role : s'execute en cas d'alarme 80% de batterie
    */
    TaskData *d = static_cast<TaskData*>(data);

	// repasser l'alarme80 à 0
	pthread_mutex_lock(&globalmutex.mtx_alarm80);
	d->my_Nav.alarm80 = false;
	pthread_mutex_unlock(&globalmutex.mtx_alarm80);

	cout << "Alarme 80%" << endl;
	pthread_mutex_lock(&globalmutex.mtx_ctrl_charge);
	d->my_Nav.ctrl_charge = 0; //ttAnalogOut(CTRL_CHARGE, true);
	pthread_mutex_unlock(&globalmutex.mtx_ctrl_charge);
	d->Next_vitesse = 80;
	d->My_PathMap.genWp(d->Position, d->FinalDest, d->Next_dest);
	pthread_mutex_lock(&globalmutex.mtx_alarm10);
	d->my_Nav.alarm10 = false;
	pthread_mutex_unlock(&globalmutex.mtx_alarm10);

}

void* systemeContinu(void* data){
    /*
    * Role : modéliser le systeme continu
    */

   	void **params = (void **)data;

    TaskData *d = static_cast<TaskData*>((TaskData*)params[0]);

	//alarm
	pthread_t t_alarm_10;	// a creer avec pthread_create
	pthread_t t_alarm_80; //

	struct timespec tp;
	sem_t*          sync_sem;
	uint32_t        task_id;
	uint32_t        starttime;
	uint32_t        elapsed_time;

	/* Get the arguments */
	sync_sem  = ((thread_args_t*)(params[1]))->semaphore;
	task_id   = ((thread_args_t*)(params[1]))->id;
	starttime = ((thread_args_t*)(params[1]))->starttime;
	uint32_t max_execution_time = MAX_EXECUTION_TIME*1000;
	uint32_t endtime = starttime + max_execution_time;


	/* Routine loop */
	while(((thread_args_t*)params[1])->run==1)
	{
		if(0 == sem_wait(sync_sem)) {

			/* Get the current time */
			if(0 == clock_gettime(CLOCK_MONOTONIC, &tp)) {

				// mettre a jours le temps d'execution
				//printf("@%d  sysCont ID=%d\n", elapsed_time, task_id);

				elapsed_time = (tp.tv_sec - starttime) * 1000 + tp.tv_nsec / 1000000;


				//----------------------------- Partie effective -------------------------
				//cas de recharge de batterie
				if ( d->my_Nav.read_ctrl_charge() == true)
				{

					d->my_Nav.recharge_batterie();
					// attendre que la batterie soit pleine
					if (d->my_Nav.read_alarm80() == false){
					}
					else
					{
						if(0 != pthread_create(&t_alarm_80, NULL, alarm_80, d)) {
								/* Print error */
								printf("Could not create thread: %d\n", errno);
								//return EXIT_FAILURE;
						}
						pthread_join(t_alarm_80, NULL);
						d->Chargement =false;
					}

				}
				
				// cas plus de batterie
				else if (d->my_Nav.read_alarm10() == true && d->Chargement == false)
				{
					d->Chargement =true;
					if(0 != pthread_create(&t_alarm_10, NULL, alarm_10, d)) {
							/* Print error */
							printf("Could not create thread: %d\n", errno);
							//return EXIT_FAILURE;
					}
					pthread_join(t_alarm_10, NULL);
				}

				d->my_Nav.compute_batterie();
				d->my_Nav.compute_orientation();
				d->my_Nav.compute_vitesse();
				d->my_Nav.position();


							
				//----------------------------- Fin partie effective ----------------------


				if (elapsed_time >= endtime)
				{
                    printf("Task %d has completed execution.\n", task_id);
                    // Exit the loop
                    ((thread_args_t*)params[1])->run=0;

                }

			}
			else
			{
				/* Print error */
				printf("Task %d could not get time: %d\n", task_id, errno);
			}
		
		}
		else {
			printf("Task %d could not wait semaphore: %d\n", task_id, errno);
		}
	}


}




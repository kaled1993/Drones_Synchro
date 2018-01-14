#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <semaphore.h>
#include <signal.h>



// Définition des couleurs

#define red   "\033[0;31m"
#define cyan  "\033[0;36m"
#define green "\033[0;32m"
#define blue  "\033[0;34m"
#define black  "\033[0;30m"
#define brown  "\033[0;33m"
#define magenta  "\033[0;35m"
#define gray  "\033[0;37m"
#define none   "\033[0m"

// Fin définition des couleurs



// Déclaration des types

typedef struct coordonnee_t coordonnee_t;
typedef struct navette_t navette_t;
typedef struct demande_t demande_t;
typedef struct drone_t drone_t;

// Fin déclaration des types



// Déclaration des fonctions des threads

void *thread_navette(void);
void *thread_expedition(void *param);
void *thread_recharge_drones(void);

// Fin déclaration des fonctions des threads



// Déclaration des fonctions utiles

int initialisation_drones(char* dronesFileName); // initialisation des drones 
int initialisation_demandes(char* demandesFileName); // initialisation des colis
int initialisation_navette(char* navetteFileName); // initialisation de la base
float get_distance(coordonnee_t coordonnees_client, coordonnee_t coordonnees_navette);
void initialisation_tableaux(); // initialise nos données de simulation
void initialisation_semaphores(); // initialise les sémaphores créées pour la synchronisation
void tri_demandes_par_priorite(); // trie les demandes du tableau par priorité dans l'ordre croissante
int calcul_temps_drone(int i, int j); // renvoie le temps entier nécessaire à une livraison à partir de l'index du drone et de l'index de la demande
int verification_drone_demande(int i); //renvoie l'index du drone dispo à partir de l'index de la demande
void occupation_slot(int i, int idx); // occupe un slot disponible ou attend la disponibilité de l'un pour l'occuper et ensuite libère
void livrer(int i, int idx); // livre un client à partir de l'index de la demande et de l'index du drone attribué
void destruction_semaphores(); // détruit les sémaphores créées pour la synchronisation

// Fin déclaration des fonctions utiles



// Déclaration des traitants de signaux

void traitantSIGINT(int num);

// Fin déclaration des traitants signaux



// Définition des types

struct coordonnee_t{
	float longitudue ;
	float latitude ; 
};

struct navette_t
{
	coordonnee_t coordonnees;
	int nbSlots;
};

struct demande_t
{
	float distanceClient;
	coordonnee_t coordonneesClient;
	unsigned int priorite;
	int taille;
	unsigned int type; // 1 pour livraison; 2 pour retour
};

struct drone_t
{
	int autonomieMax;
	int autonomieRestante;
	unsigned int statut; // 1 pour disponible; 2 pour occupé
	float vitesse;
	int vitesseRecharge;
	int capacite;
};

// Fin définition des types



// Déclaration des variables globales

#define NB_DEMANDES_MAX 100
#define NB_DRONES_MAX 50
int NB_DEMANDES; 
int NB_DRONES ;  
navette_t navette;
demande_t demandes[NB_DEMANDES_MAX]; 
drone_t drones[NB_DRONES_MAX];

// Fin déclaration des variables globales



// Déclaration des sémaphores

sem_t mutexAccesDrones, mutexAccesSlots, mutexLiberationDrone, mutexLiberationSlot;

// Fin déclaration des sémaphores



// Définition des fonctions utiles

int initialisation_navette(char* navetteFileName)
{	
	FILE* pFile;
	char * line = NULL;
	size_t len = 0;	
	if(navetteFileName)
	{
		pFile= fopen(navetteFileName,"r");
		if(pFile)
		{	
		   	if(getline(&line, &len, pFile) != EOF) 
			{
		     	sscanf(line, "%d,%f,%f", &navette.nbSlots ,&navette.coordonnees.latitude,&navette.coordonnees.longitudue);
			}
		}
		else
		{
		 	perror("Erreur ouverture fichier navette");
		}
	}
	fclose(pFile);
}

int initialisation_drones(char* droneFileName)
{	
	FILE * pFile;
	char * line = NULL;
	size_t len = 0;	
	int nbDrones = 0 ;
	if(droneFileName)
	{
		pFile = fopen(droneFileName, "r");
		if(pFile)
		{	
		   	while((getline(&line, &len, pFile) != EOF) && (&drones[nbDrones]))
		 	{	
			 	drone_t tmpDrone;
		     	sscanf(line, "%d,%f,%d,%d",&tmpDrone.autonomieMax,&tmpDrone.vitesse,&tmpDrone.vitesseRecharge,&tmpDrone.capacite);
		     	tmpDrone.statut = 1;
				tmpDrone.autonomieRestante = tmpDrone.autonomieMax;
				tmpDrone.vitesse = tmpDrone.vitesse / 3600;
				drones[nbDrones] = tmpDrone;
				nbDrones++;			
		   	}
		}
		else
		{
			perror("Erreur ouverture fichier drones");
		}
	}
	fclose(pFile);
	return nbDrones;
}

int initialisation_demandes(char* demandesFileName ) 
{	
	FILE * pFile;
	char * line = NULL;
	size_t len = 0;	
	int nbDemande = 0 ;
	if(demandesFileName)
	{
		pFile= fopen(demandesFileName, "r");
		if(pFile)
		{
			int i, capaciteMax = 0;
			float porteeMax = 0;
			for(i = 0; i < NB_DRONES; i++)
			{
				float tmp = drones[i].autonomieMax * drones[i].vitesse / 2;
				if (porteeMax < tmp)
					porteeMax = tmp;
				if (drones[i].capacite > capaciteMax)
					capaciteMax = drones[i].capacite;
			}
		   	while((getline(&line, &len, pFile) != EOF) && (&demandes[nbDemande])) 
			{
				demande_t tmpDemande;
		     	sscanf(line, "%f,%f,%d,%d,%d",&tmpDemande.coordonneesClient.latitude,&tmpDemande.coordonneesClient.longitudue,&tmpDemande.priorite,&tmpDemande.taille,&tmpDemande.type);
				tmpDemande.distanceClient = get_distance(tmpDemande.coordonneesClient,navette.coordonnees);
				if ((tmpDemande.distanceClient <= porteeMax) && (tmpDemande.taille <= capaciteMax))
				{
					demandes[nbDemande] = tmpDemande;
					nbDemande++;
				}
    		}
		}
		else
		{
			perror("Erreur ouverture fichier demandes");
		}
	}
	fclose(pFile);
	return nbDemande;
}

float get_distance(coordonnee_t clientCoord,coordonnee_t navetteCoord)
{	
	return sqrt(pow(clientCoord.longitudue - navetteCoord.longitudue, 2) + pow(clientCoord.latitude - navetteCoord.latitude, 2)) * 10;
}

void initialisation_tableaux()
{
	initialisation_navette("navette.txt");
	NB_DRONES = initialisation_drones("drones.txt");
	printf("\t%s%d drones initialises...\n", green, NB_DRONES);
	NB_DEMANDES = initialisation_demandes("demandes.txt");
	printf("\t%s%d demandes initialisees...\n", green, NB_DEMANDES);
}


void initialisation_semaphores()
{
	printf("\t%sInitialisation semaphores...\n", brown);
	if (sem_init(&mutexAccesDrones, 0, 1) == -1)
		perror("Erreur initialisation mutexAccesDrones");
	if (sem_init(&mutexAccesSlots, 0, 1) == -1)
		perror("Erreur initialisation mutexAccesSlots");
	if (sem_init(&mutexLiberationDrone, 0, 0) == -1)
		perror("Erreur initialisation mutexLiberationDrone");
	if (sem_init(&mutexLiberationSlot, 0, 0) == -1)
		perror("Erreur initialisation mutexLiberationSlot");
}

void tri_demandes_par_priorite()
{
   	int i, j;
   	for (i = 1; i < NB_DEMANDES; ++i) 
   	{
       	demande_t elem = demandes[i];
       	for (j = i; j > 0 && demandes[j-1].priorite > elem.priorite; j--)
       		demandes[j] = demandes[j-1];
       	demandes[j] = elem;
   	}
}

int calcul_temps_drone(int i, int j)
{
	return (int)ceil(2 * demandes[j].distanceClient / drones[i].vitesse);
}

int verification_drone_demande(int i)
{
	int j, ret = -1;
	for (j = NB_DRONES; j >= 0; j--)
	{
		int time = calcul_temps_drone(j, i);
		if ((time < drones[j].autonomieRestante) && (drones[j].statut == 1) && (drones[j].capacite >= demandes[i].taille))
			ret = j;
	}
	return ret;
}

void occupation_slot(int i, int idx)
{
	sem_wait(&mutexAccesSlots); // attente signal accès slots
	printf("\t\t\t%sExpedition %d : Verification disponibilite de slot avec drone %d...\n", magenta, i+1, idx+1);
	if (navette.nbSlots = 0)
	{
		sem_post(&mutexAccesSlots); // émission signal accès slots
		printf("\t\t\t%sExpedition %d : Aucun slot disponible, attente disponibilite avec drone %d...\n", magenta, i+1, idx+1);
		sem_wait(&mutexLiberationSlot); // attente signal libération slot
	}
	else
	{
		printf("\t\t\t%sExpedition %d : Occupation d'un slot avec drone %d...\n", none, i+1, idx+1);
		navette.nbSlots--;
		sem_post(&mutexAccesSlots); // émission signal accès slots
		usleep(5000000);
		sem_wait(&mutexAccesSlots); // attente signal accès slots 
		printf("\t\t\t%sExpedition %d : Liberation du slot occupe avec drone %d...\n", none, i+1, idx+1);
		navette.nbSlots++;
		sem_post(&mutexAccesSlots); // émission signal accès slots
	}
}

void livrer(int i, int idx)
{
	int time = calcul_temps_drone(idx, i);
	if (demandes[i].type == 1)
		printf("\t\t\t%sExpedition %d : La livraison avec drone %d prendra %d...\n", cyan, i+1, idx+1, time);
	else
		printf("\t\t\t%sExpedition %d : La recuperation avec drone %d prendra %d...\n", cyan, i+1, idx+1, time);
	usleep(time*1000000);
	drones[idx].autonomieRestante -= time;
	drones[idx].statut = 1;
}

void destruction_semaphores()
{
	printf("\t%sDestruction semaphores...\n", brown);
	if (sem_destroy(&mutexAccesDrones) == -1)
		perror("Erreur destruction mutexAccesDrones");
	if (sem_destroy(&mutexAccesSlots) == -1)
		perror("Erreur destruction mutexAccesSlots");
	if (sem_destroy(&mutexLiberationDrone) == -1)
		perror("Erreur destruction mutexLiberationDrone");
	if (sem_destroy(&mutexLiberationSlot) == -1)
		perror("Erreur destruction mutexLiberationSlot");
	printf("%s\n", none);
}

// Fin définition des fonctions utiles



// Définition des fonctions des threads

void *thread_navette(void)
{
	int i, params[NB_DEMANDES];
	pthread_t threads[NB_DEMANDES], thdRechargeDrones;
	printf("\t\t%sNavette : Lancement du rechargement des drones...\n", gray);
	if (pthread_create(&thdRechargeDrones, NULL, thread_recharge_drones, NULL))
		perror("Erreur creation thread recharge drones");
	printf("\t\t%sNavette : Tri des demandes par priorite...\n", gray);
	tri_demandes_par_priorite();
	for (i = 0; i < NB_DEMANDES; i++)
    {
    	params[i] = i;
		printf("\t\t%sNavette : Lancement de l'expedition %d...\n", gray, i+1);
	  	if (pthread_create(threads+i, NULL, thread_expedition, params+i))
	  	    perror("Erreur creation thread expedition");
    }
	printf("\t\t%sNavette : Attente de la fin des expeditions...\n", gray);
    for (i = 0; i < NB_DEMANDES; i++)
    {
    	pthread_join(threads[i], NULL);
    }
    printf("\t\t%sNavette : Fin de toutes les expeditions...\n", gray);
    pthread_cancel(thdRechargeDrones);

    pthread_exit(NULL);
}

void *thread_expedition(void *param)
{
	int *ptr_i = (int *)param;
	int i = *ptr_i;
	while(1)
	{
		sem_wait(&mutexAccesDrones); // attente signal accès drones
		printf("\t\t\t%sExpedition %d : Verification disponibilite de drone...\n", magenta, i+1);
		int idx = verification_drone_demande(i);
		if (idx == -1)
		{
			sem_post(&mutexAccesDrones); // émission signal accès drones
			printf("\t\t\t%sExpedition %d : Aucun drone disponible, attente disponibilite...\n", none, i+1);
			sem_wait(&mutexLiberationDrone); // attente signal libération drone
		}
		else
		{
			drones[idx].statut = 2;
			sem_post(&mutexAccesDrones); // émission signal accès drones
			printf("\t\t\t%sExpedition %d : Choix drone %d...\n", magenta, i+1, idx+1);
			if (demandes[i].type == 1)
			{
				occupation_slot(i, idx);
				printf("\t\t\t%sExpedition %d : Lancement livraison avec drone %d...\n", none, i+1, idx+1);
			}
			else
			{
				printf("\t\t\t%sExpedition %d : Lancement recuperation avec drone %d...\n", none, i+1, idx+1);
			}
			livrer(i, idx);
			if (demandes[i].type == 2)
			{
				printf("\t\t\t%sExpedition %d : Recuperation effectue avec drone %d...\n", none, i+1, idx+1);
				occupation_slot(i, idx);
			}
			else
			{
				printf("\t\t\t%sExpedition %d : Livraison effectuee avec drone %d...\n", none, i+1, idx+1);
			}
			sem_post(&mutexLiberationDrone); // émission signal libération drone
			pthread_exit(NULL);
		}
	}
}

void *thread_recharge_drones(void)
{
	while(1)
	{
		int i;
		for (i = 0; i < NB_DRONES; i++)
	    {
	    	if (drones[i].statut == 1 )
	    	{
	    		if (drones[i].autonomieRestante < drones[i].autonomieMax)
	    			drones[i].autonomieRestante += drones[i].vitesseRecharge;
	    		else
	    			sem_post(&mutexLiberationDrone); //émission signal libération drone
	    	}
	    }
	    usleep(1000000);
	}
}

// Fin définition des fonctions des threads



// Définition des traitants signaux

void traitantSIGINT(int num) {
  	if (num != SIGINT)
    	perror("Probleme sur SIGINT...");
    printf("\t\n");
    destruction_semaphores();
    exit(EXIT_SUCCESS);
}

// Fin définition des traitants signaux



int main(int argc, char *argv[])
{
	initialisation_tableaux();
	initialisation_semaphores();
	signal(SIGINT,traitantSIGINT);

	pthread_t thdNavette;
	printf("\t%sLancement de la navette...\n", blue);
	if (pthread_create(&thdNavette, NULL, thread_navette, NULL))
		perror("Erreur creation thread navette");

	pthread_join(thdNavette, NULL);
	printf("\t%sArret de la navette...\n", blue);

	destruction_semaphores();

	exit(EXIT_SUCCESS);
}

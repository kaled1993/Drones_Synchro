Projet Livraison par drones 

Réalisé par Krifa Khaled

Le projet ce compose de  6 fichiers :

	+ main.c : contient le programme  en c
 
	+ drones.txt : contient les différentes données des drones
		Chaque ligne contient les données d'un drone distinct : autonomie, vitesse, vitesse de recharge, capacité
			-> autonomie : autonomie maximale du drone
			-> vitesse : vitesse de déplacement du drone
			-> vitesse de recharge : plus la vitesse de recharge est élevée, moins le drone prend de temps pour se recharger
			-> capacité : représente la taille maximale de colis que le drone peut prendre

	+ navette.txt : contient les données de la navette où il y a initialement les drones et les colis 
		Les données de la navette sont insérées comme suit : nombre des slots, coordonnées GPS telles que latitude et longitude  
			-> nombre des slots : représente le nombres de positions où les drones peuvent prendre ou déposer des colis 
			-> coordonnées : la position exacte de la navette (dans notre exemple la position de la navette est la position de l'UTBM latitude = '47.58841'et longitude ='6.8655373')

	+ demandes.txt : contient les données des demandes (livraisons ou retours de colis) 
		Chaque ligne contient les données suivantes : coordonnées GPS, la priorité, la taille, le type
			-> coordonnées GPS : représente la position où le coli doit être livré 
			-> l'ordre d'exécution de la tâche livraison se fait par priorité 
			-> la taille : est une variable pour simuler la grandeur d'un colis
			-> le type : 1 pour livraison, 2 pour retour de colis


	+ un fichier makefile pour faciliter la compilation du programme

	+ lanceur.sh pour lancer la compilation suivie de l'exécution du programme
		S'assurer d'avoir le droit d'exécution du fichier puis le lancer.

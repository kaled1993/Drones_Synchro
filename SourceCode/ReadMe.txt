Projet Livraison par drones 

R�alis� par Krifa Khaled

Le projet ce compose de  6 fichiers :

	+ main.c : contient le programme  en c
 
	+ drones.txt : contient les diff�rentes donn�es des drones
		Chaque ligne contient les donn�es d'un drone distinct : autonomie, vitesse, vitesse de recharge, capacit�
			-> autonomie : autonomie maximale du drone
			-> vitesse : vitesse de d�placement du drone
			-> vitesse de recharge : plus la vitesse de recharge est �lev�e, moins le drone prend de temps pour se recharger
			-> capacit� : repr�sente la taille maximale de colis que le drone peut prendre

	+ navette.txt : contient les donn�es de la navette o� il y a initialement les drones et les colis 
		Les donn�es de la navette sont ins�r�es comme suit : nombre des slots, coordonn�es GPS telles que latitude et longitude  
			-> nombre des slots : repr�sente le nombres de positions o� les drones peuvent prendre ou d�poser des colis 
			-> coordonn�es : la position exacte de la navette (dans notre exemple la position de la navette est la position de l'UTBM latitude = '47.58841'et longitude ='6.8655373')

	+ demandes.txt : contient les donn�es des demandes (livraisons ou retours de colis) 
		Chaque ligne contient les donn�es suivantes : coordonn�es GPS, la priorit�, la taille, le type
			-> coordonn�es GPS : repr�sente la position o� le coli doit �tre livr� 
			-> l'ordre d'ex�cution de la t�che livraison se fait par priorit� 
			-> la taille : est une variable pour simuler la grandeur d'un colis
			-> le type : 1 pour livraison, 2 pour retour de colis


	+ un fichier makefile pour faciliter la compilation du programme

	+ lanceur.sh pour lancer la compilation suivie de l'ex�cution du programme
		S'assurer d'avoir le droit d'ex�cution du fichier puis le lancer.

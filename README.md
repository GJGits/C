# C
Questo repository contiene vari progetti utili alla comprensione del
linguaggio C. Ogni sotto cartella rappresenta un progetto a se stante.
Per l'utilizzo e il test dei sorgenti è stata configurata un'apposita immagine docker

## Installazione con Docker 
Per poter utilizzare l'applicazione occore seguire i seguenti passaggi:
1. Installare Docker CE se non precedentemente fatto
2. Posizionarsi nella cartella C 
3. Eseguire uno dei seguenti comandi:
	1. Per lanciare un server: ./app-dock `<project-name> -s <param1> <param2> ... <paramN>`
	2. Per lanciare un client: ./app-dock `<project-name> -c <param1> <param2> ... <paramN>`
> **NB:** lo script contiene comandi `sudo` quindi va eseguito su macchina Linux

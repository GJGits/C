# C
Questo repository contiene vari progetti utili alla comprensione del
linguaggio C. Ogni sotto cartella rappresenta un progetto a se stante.
Per l'utilizzo e il test dei sorgenti è stata configurata un'apposita immagine docker

## Struttura di un progetto
Per inizializzare un nuovo progetto creare all'interno della cartella del ropository una nuova directory, di conseguenza creare 
al suo interno le seguenti subdirectory:
* **src:** cartella che contiene tutti i sorgenti dell'applicazione, importante ricordare che non devono essere presenti due main, quest
potrebbe capitare qualora ci fossero dei test.
* **headers:** conterrà tutti i file di header locali, se invece si volesse definire un header globale, condivisibile quindi con
altri progetti, esso dovrà essere inserito nella cartella `global-headers` presente all'interno del repository. Questa cartella per ora
prevede solamente file con estensione `.h` di conseguenza non contiene al suo interno la definizione dei metodi. Per poter utilizzare i
file di libreria in esso presenti copiare i file con estensione `.c` all'interno della cartella `src` di cui sopra.
* **test:** cartella per ora inutilizzata, tenuta per avere una struttura dei progetti estensibile e simile a quella utilizza per i progetti
in C++ 

## Installazione con Docker 
Per poter utilizzare l'applicazione occore seguire i seguenti passaggi:
1. Installare Docker CE se non precedentemente fatto
2. Posizionarsi nella cartella C 
3. Eseguire uno dei seguenti comandi:
	1. Per lanciare un server: ./app-dock `<project-name> -s <param1> <param2> ... <paramN>`
	2. Per lanciare un client: ./app-dock `<project-name> -c <param1> <param2> ... <paramN>`
> **NB:** lo script contiene comandi `sudo` quindi va eseguito su macchina Linux

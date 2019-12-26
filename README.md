# PD1
Questo repository contiene vari progetti relativi ai laboratori del corso: *Programmazione distribuita I (2018/2019)* tenuto presso il [Politecnico di Torino](https://www.polito.it/). 

## Creazione di un progetto
Per creare un progetto in maniera automatica è possibile utilizzare lo script `crapp` presente all'interno del repository. Per utilizzarlo:
Eseguire uno dei seguenti comandi:
	1. Per creare un applicativo server: ./crapp `<project-name> -s [-tcp|-udp]` attualmente  		   l'unico protocollo supportato è tcp quindi il comando diventa: `<project-name> -s -tcp`
	2. Per creare un applicativo client: ./crapp `<project-name> -c`

## Struttura di un progetto


## Utilizzare app con Docker 
Per poter utilizzare l'applicazione con docker occore seguire i seguenti passaggi:
1. Installare [Docker CE](https://docs.docker.com/install/linux/docker-ce/ubuntu/) se non precedentemente fatto 
2. Eseguire uno dei seguenti comandi:
	1. Per lanciare un server: ./app-dock `<project-name> -s <param1> <param2> ... <paramN>`
	2. Per lanciare un client: ./app-dock `<project-name> -c <param1> <param2> ... <paramN>`
> **NB:** Non spostare gli script dalla loro relativa cartella e non modificare la struttura del progetto

## Configurazione di rete
Ogni applicativo risiede all'interno di un container che espone la porta 1500. Il binding della porta lato host viene fatto in due modi
differenti a seconda che l'applicativo ospiti un client o un server. Se l'applicazione ospita un server il binding viene effettuato sulla
porta 3000, non è possibile quindi al momento avere diversi server che lavorano in parallelo. Per quanto riguarda invece un client la porta
di binding viene scelta in maniera random, questa infatti non ha bisogno di essere nota a priori in quanto le connessioni vengono aperte dai
client. In definitiva un applicativo client per contattare un server dovrà specificare due parametri: `<server address> 3000`
> **NB:** se si adopera docker su macchina linux i container sono raggiungibili tramite l'indirizzo di localhost: `127.0.0.1` 

## Utilizzare app in locale
Per poter utilizzare l'applicazione in locale occore seguire i seguenti passaggi:
1. Installare `gcc` 
2. Eseguire uno dei seguenti comandi:
	1. Per lanciare un server: ./app-local `<project-name> -s <param1> <param2> ... <paramN>`
	2. Per lanciare un client: ./app-local `<project-name> -c <param1> <param2> ... <paramN>`
> **NB:** Non spostare gli script dalla loro relativa cartella e non modificare la struttura del progetto

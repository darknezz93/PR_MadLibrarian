#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <list>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>

#define ROOT 0
#define MSG_TAG 100
#define BUFF_SIZE 4
#define NUMB_OF_MPC 3

using namespace std;

int tid;
int size;
int customersCount;
int myLamp;
int msgSnd[BUFF_SIZE];
int msgRcv[BUFF_SIZE];
int mpcTab[NUMB_OF_MPC];
int priorities[100]; //tablica procesow
char processor[100];
list<int> processesIds;
MPI_Status status;

bool czyMogeWejsc;
int changeStateEnable; // 
int sleeping; // czy ubiegamy sie o sesje
int mpcTabFull; // czy jest jeszcze miejsce w tablicy
int inSection; //czy jestem w sekcji
void readAnswer(int id, int numOfReaders, int answer);



void create(){
	
	srand(time(0));
	customersCount = rand()%(tid+1) + 7;
	printf("[%d](%d)create | custCoun = %d\n", myLamp, tid, customersCount);
	for(int i  = 0; i < size+1; i++)
	{
		priorities[i] = 0;
	}
	priorities[tid] = 1; //sam sobie zezwalam
	msgSnd[0] = tid;
	msgSnd[1] = customersCount;
	msgSnd[3] = myLamp;
	czyMogeWejsc = false;
	inSection = 0;
	printf("[%d](%s)Librarian o id: %d i liczbie klientow: %d\n", myLamp, processor, tid, customersCount);
	//cout<<"Librarian o id: "<<id<<" i liczbie klientow: "<<customersCount<<endl; 
}


void sendAll(int param) {
	//rozsy³anie requestów
	for (int i = 0; i<size; i++){
		if (i != tid){
			msgSnd[2] = param;
			MPI_Send(msgSnd, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
			myLamp++;
		}
	}
}

void canEnter() {
	//sprawdzenie tablicy priorytetów
	//printf("canEnter\n");
	czyMogeWejsc = true;
	for (int i = 0; i<size; i++){
		if (priorities[i] == 0){
			czyMogeWejsc = false;
			break;
		}
	}
}

/*void waitForAnswears() {
	//oczekiwanie odpowiedzi
	for (int it = 1; it<size; it++){ //oczekuje na size-1 odpowiedzi
		//MPI_Status status;
		MPI_Recv(msgRcv, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		readAnswer(msgRcv[0], msgRcv[1], msgRcv[2]);
	}
}*/


int mpcTabIsFull()
{
	for(int i=0; i<NUMB_OF_MPC; i++){
		if (mpcTab[i]==0) 
			return 0;
	}
	return 1;
}

void accessMPC() {
	//dostêp do MPC
	printf("[%d](%d): ", myLamp, tid);
	for(int i=0; i<size; i++)
	{
		printf(" %d", priorities[i]);
	}
	cout<<endl;

	if (czyMogeWejsc){
		//SEKCJA KRYTYCZNA
		printf("[%d]Wszedlem(%d)--------------------------------------\n", myLamp, tid);
		inSection = 1;
		while(mpcTabIsFull()){printf("MPC is full");}
		sleep(3);
		printf("[%d]Wyszeldem(%d)==========================\n", myLamp, tid);
		sendAll(100);

	}else{
		printf("Nie wszedlem(%d)\n", tid);
	}
}



void readAnswer(int id, int numbOfReaders, int answer, int lamport){ 
	//printf("Metoda ReadAnswer()\n");
	printf("[%d](%d |NC: %d)Odpowiedz od: %d |NC: %d |CD: %d |L: %d\n",
		       	myLamp, tid, customersCount, id, numbOfReaders, answer, lamport);
	if (myLamp < lamport) myLamp = lamport+1;
	if (inSection == 1 and mpcTabIsFull()) {
		int msg[BUFF_SIZE];
		msg[0] = tid;
		msg[1] = 300; //300 - brak zgody ( w pelnej sekcji )
		msg[2] = customersCount;
		MPI_Send(msg, BUFF_SIZE, MPI_INT, id, MSG_TAG, MPI_COMM_WORLD);
		myLamp++;
		sleep(1);
		return;
	}
	if (answer == 300){ // 300 - brak zgody (proces w pelnej sesji)
		priorities[id] = 0;
	}
	if (answer == 100){ // 100 - kod dla odpowiedzi "agree"
		priorities[id] = 2; //2 - wartosc dla odpowiedzi "agree"
		return;
	}

	if (answer == 200 and numbOfReaders > customersCount){
		priorities[id] = 0; // 0 - brak zezwolenia (przegrana walka)
		return;
	}
	if (answer == 200 and numbOfReaders < customersCount){
		priorities[id] = 1; //1 - zezwolenie (wygrana walka)
		return;
	}
	if (answer == 200 and numbOfReaders == customersCount)
		if(tid > id)
		{
			priorities[id] = 1;
			return;
		}else{
			priorities[id] = 0;
			return;
		}
}

bool canContinue = false;

void *listener(void *)
{
	//int x = (int) *test;
	printf("Watek(%d)\n", tid);
	while(1)
	{
		MPI_Recv(msgRcv, BUFF_SIZE, MPI_INT, MPI_ANY_SOURCE, 
				MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		//waitForAnswears();
		readAnswer(msgRcv[0], msgRcv[1], msgRcv[2], msgRcv[3]);

	}
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	int len;

	//MPI_Status status;

	MPI::Init(argc, argv);
	size = MPI::COMM_WORLD.Get_size();
	tid = MPI::COMM_WORLD.Get_rank();
	MPI::Get_processor_name(processor, len);	

	//printf("(%s)Librarian o id: %d i licznie klientow: %d\n", processor, tid, customersCount);

	pthread_t handler;
	//pthread_create(&handler, NULL, listener, NULL);

	pthread_create(&handler, NULL, listener, NULL);
	while(1){
		//int priorities[size]; //0 - brak zgody 1 - wygrana walka 2 - zezwolenie
		create();
		//printf("Hello! My name is %s (%d of %d)\n", processor, tid, size);

		sendAll(200); //wyslanie requesto
		
		//for(int i=0; i<size; i++){ printf("%d ", priorities[i]); printf("[%d]\n", tid); }
		//pthread_create(&handler, NULL, listener, NULL);

		while(!czyMogeWejsc){
			canEnter();
		}

		accessMPC();
		
		//printf("%d -- %d\n", tid, test);
	}
	pthread_cancel(handler);

	MPI::Finalize();
}

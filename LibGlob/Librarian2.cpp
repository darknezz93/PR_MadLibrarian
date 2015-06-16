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
#define NUMB_OF_MPC 3

using namespace std;

 	int tid;
	int size;
	int customersCount;
	int msg[3];
	int mpcTab[NUMB_OF_MPC];

	list<int> processesIds;
	
	bool czyMogeWejsc;

	void readAnswer(int id, int numOfReaders, int answer, int prio[]);
	
	

void create(int id, int size, int priorities[], char maszyna[]){
	srand(time(0));
	tid = id;
	size = size;
	customersCount = rand()%(id+1) + 7;
	for(int i  = 0; i < size+1; i++)
	{
		 priorities[i] = 0;
	}

	priorities[id] = 1; //sam sobie zezwalam
	msg[0] = id;
	msg[1] = customersCount;
	czyMogeWejsc = false;
	printf("(%s)Librarian o id: %d i licznie klientow: %d\n", maszyna, id, customersCount);
	//cout<<"Librarian o id: "<<id<<" i liczbie klientow: "<<customersCount<<endl; 
}


void sendRequests() {
	//rozsy³anie requestów
	for (int i = 0; i<size; i++){
		if (i != tid){
			msg[2] = 200;
			MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
		}
	}
}

void canEnter(int priorities[]) {
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

void waitForAnswears(int prio[]) {
	//oczekiwanie odpowiedzi
	for (int i = 1; i<size; i++){ //oczekuje na size-1 odpowiedzi
		MPI_Status status;
		MPI_Recv(msg, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		readAnswer(msg[0], msg[1], msg[2], prio);
	}
}

//int test;

void accessMPC(int priorities[]) {
	//dostêp do MPC
	if (czyMogeWejsc){
		printf("Wszedlem(%d)--------------------------------------\n", tid);
	}else{
		printf("Nie wszedlem(%d)\n", tid);
	}
	printf("(%d): ", tid);
	for(int i=0; i<size; i++)
	{
		printf(" %d", priorities[i]);
	}
	cout<<endl;
}



void readAnswer(int id, int numbOfReaders, int answer, int priorities[]){ 
	//printf("Metoda ReadAnswer()\n");
	//printf("(%d | %d)Odpowiedz od: %d | %d | %d\n", tid, customersCount, id, numbOfReaders, answer);
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
	if (numbOfReaders == customersCount and tid > id)
	{
		priorities[id] = 1;
		return;
	}
	else{
		priorities[id] = 0;
		return;
	}
}


void *listener(void *)
{
	//int x = (int) *test;
	printf("Watek(%d)\n", tid);
	
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	int len;
	int test = 2;
	char processor[100];

	//MPI_Status status;

	MPI::Init(argc, argv);
	size = MPI::COMM_WORLD.Get_size();
	tid = MPI::COMM_WORLD.Get_rank();
	MPI::Get_processor_name(processor, len);
	
	pthread_t handler;
	pthread_create(&handler, NULL, listener, NULL);
	sleep(1);

	int priorities[size]; //0 - brak zgody 1 - wygrana walka 2 - zezwolenie
	create(tid, size, priorities, processor);
	//printf("Hello! My name is %s (%d of %d)\n", processor, tid, size);
	
	sendRequests();
	waitForAnswears(priorities);
	//for(int i=0; i<size; i++){ printf("%d ", priorities[i]); printf("[%d]\n", tid); }
	canEnter(priorities);
	accessMPC(priorities);
	//printf("%d -- %d\n", tid, test);

	MPI::Finalize();
}

#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <list>
#include <ctime>
#include <cstdlib>
#include <unistd.h>

#define ROOT 0
#define MSG_TAG 100
#define NUMB_OF_LIBR 5

using namespace std;

 	int tid;
	int size;
	int customersCount;
	int msg[3];
	list<int> processesIds;
	//int priorities[NUMB_OF_LIBR]; //[0] brak zgody [1] wygrana walka [2] zezwolenie
    	//list<MPC> freeMPCs;
	bool czyMogeWejsc;

	//void create(int id, int size, int priorities);	
	//void sendRequests();
	//void canEnter();
	//void waitForAnswears();
	void readAnswer(int id, int numOfReaders, int answer, int prio[]);
	//void accessMPC(int prio[]);
	

void create(int id, int size, int priorities[]){
	srand(time(0));
	tid = id;
	size = size;
	customersCount = rand()%(id+1) + 7;
	for(int i  = 0; i < size+1; i++)
	{
		 priorities[i] = 0;
	}

	// this->priorities[];
	priorities[id] = 1; //sam sobie zezwalam
	msg[0] = id;
	msg[1] = customersCount;
	czyMogeWejsc = false;
	cout<<"Librarian o id: "<<id<<" i liczbie klientow: "<<customersCount<<endl; 
}


void sendRequests() {
	//rozsy�anie request�w
	for (int i = 0; i<size; i++){
		if (i != tid){
			msg[2] = 200;
			MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
		}
	}
}

void canEnter(int priorities[]) {
	//sprawdzenie tablicy priorytet�w
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

void accessMPC(int priorities[]) {
	//dost�p do MPC
	if (czyMogeWejsc){
		printf("Weszlem(%d)--------------------------------------\n", tid);
	}else{
		printf("Nie weszlem(%d)\n", tid);
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


int main(int argc, char **argv)
{
	int  len;
	char processor[100];

	//MPI_Status status;

	MPI::Init(argc, argv);
	size = MPI::COMM_WORLD.Get_size();
	tid = MPI::COMM_WORLD.Get_rank();
	MPI::Get_processor_name(processor, len);
	
	int priorities[size];
	create(tid, size, priorities);
	//printf("Hello! My name is %s (%d of %d)\n", processor, tid, size);
	
	sendRequests();
	waitForAnswears(priorities);
	//for(int i=0; i<size; i++){ printf("%d ", priorities[i]); printf("[%d]\n", tid); }
	canEnter(priorities);
	accessMPC(priorities);
	

	MPI::Finalize();
}

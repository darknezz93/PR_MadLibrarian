#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <list>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include "MPC.cpp"
#define ROOT 0
#define MSG_TAG 100

using namespace std;

 	int id;
	int size;
	int customersCount;
	int msg[3];
	list<int> processesIds;
	int priorities[]; //[0] brak zgody [1] wygrana walka [2] zezwolenie
    list<MPC> freeMPCs;
	bool czyMogeWejsc;

	void create(int id, int size);	
	void sendRequests();
	void canEnter();
	void waitForAnswears();
	void readAnswer(int id, int numOfReaders, int answer);
	void accessMPC();


void create(int id, int size){
	srand(time(0));
	id = id;
	size = size;
	this->customersCount = rand()%(id+1) + 7;
	for(int i  = 0; i < size+1; i++)
	{
		 this->priorities[i] = 0;
	}

	// this->priorities[];
	this->priorities[id] = 1; //sam sobie zezwalam
	this->msg[0] = id;
	this->msg[1] = this->customersCount;
	this->czyMogeWejsc = false;
	cout<<"Librarian o id: "<<id<<" i liczbie klientow: "<<this->customersCount<<endl; 
}


void sendRequests() {
	//rozsy³anie requestów
	for (int i = 0; i<this->size; i++){
		if (i != this->id){
			this->msg[2] = 200;
			MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
		}
	}
}

void canEnter() {
	//sprawdzenie tablicy priorytetów
	this->czyMogeWejsc = true;
	for (int i = 0; i<this->size; i++){
		if (this->priorities[i] == 0){
			this->czyMogeWejsc = false;
			break;
		}
	}
}

void waitForAnswears() {
	//oczekiwanie odpowiedzi
	for (int i = 1; i<this->size; i++){ //oczekuje na size-1 odpowiedzi
		MPI_Status status;
		MPI_Recv(msg, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		this->readAnswer(this->msg[0], this->msg[1], this->msg[2]);
	}
}

void accessMPC() {
	//dostêp do MPC
	if (this->czyMogeWejsc){
		printf("Weszlem(%d)--------------------------------------\n", this->id);
	}else{
		printf("Nie weszlem(%d)\n", this->id);
	}
	printf("(%d): ", this->id);
	for(int i=0; i<this->size; i++)
	{
		printf(" %d", this->priorities[i]);
	}
	cout<<endl;
}



void readAnswer(int id, int numbOfReaders, int answer){ 
	//printf("Metoda ReadAnswer()\n");
	printf("(%d | %d)Odpowiedz od: %d | %d | %d\n", this->id, this->customersCount, id, numbOfReaders, answer);
	if (answer == 100){ // 100 - kod dla odpowiedzi "agree"
		this->priorities[id] = 2; //2 - wartosc dla odpowiedzi "agree"
		return;
	}
	if (answer == 200 and numbOfReaders > this->customersCount){
		this->priorities[id] = 0; // 0 - brak zezwolenia (przegrana walka)
		return;
	}
	if (answer == 200 and numbOfReaders < this->customersCount){
		this->priorities[id] = 1; //1 - zezwolenie (wygrana walka)
		return;
	}
	if (numbOfReaders == this->customersCount and id < this->id)
	{
		this->priorities[id] = 1;
		return;
	}
	else{
		this->priorities[id] = 0;
		return;
	}
}


int main(int argc, char **argv)
{
	int tid, size, len;
	char processor[100];
	//MPI_Status status;

	MPI::Init(argc, argv);
	size = MPI::COMM_WORLD.Get_size();
	tid = MPI::COMM_WORLD.Get_rank();
	MPI::Get_processor_name(processor, len);

	create(tid, size);
	printf("Hello! My name is %s (%d of %d)\n", processor, tid, size);
	
	sendRequests();
	waitForAnswears();
	canEnter();
	accessMPC();
	

	MPI::Finalize();
}

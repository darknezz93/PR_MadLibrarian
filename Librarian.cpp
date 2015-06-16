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

class Librarian {
 
 private:
 	int id;
	int size;
	int customersCount;
	int msg[2];
	list<int> processesIds;
	int priorities[]; //[0] brak zgody [1] wygrana walka [2] zezwolenie
    list<MPC> freeMPCs;
	bool czyMogeWejsc;

 public:
	Librarian(int id, int size);	
	void sendRequests();
	void canEnter();
	void waitForAnswears();
	void readAnswer(int id, int numOfReaders, int answer);
	void accessMPC();

};

Librarian::Librarian(int id, int size){
	 srand(time(0));
     this->id = id;
	 this->size = size;
	 this->customersCount = rand()%(id+1) + 7;
	 this->priorities[size];
	 this->priorities[id] = 1; //sam sobie zezwalam
	 this->msg[0] = id;
	 this->msg[1] = this->customersCount;
	 this->czyMogeWejsc = false;
	 cout<<"Librarian o id: "<<id<<" i liczbie klientow: "<<this->customersCount<<endl; 
}


void Librarian::sendRequests() {
	//rozsy³anie requestów
	for (int i = 0; i<this->size; i++){
		if (i != this->id){
			this->msg[2] = 200;
			MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
		}
	}
}

void Librarian::canEnter() {
	//sprawdzenie tablicy priorytetów
	this->czyMogeWejsc = true;
	for (int i = 0; i<this->size; i++){
		if (this->priorities[i] == 0){
			this->czyMogeWejsc = false;
			break;
		}
	}
}

void Librarian::waitForAnswears() {
	//oczekiwanie odpowiedzi
	for (int i = 1; i<this->size; i++){ //oczekuje na size-1 odpowiedzi
		MPI_Status status;
		MPI_Recv(msg, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		this->readAnswer(this->msg[0], this->msg[1], this->msg[2]);
	}
}

void Librarian::accessMPC() {
	//dostêp do MPC
	if (this->czyMogeWejsc){
		printf("Weszlem(%d)\n", this->id);
	}else{
		printf("Nie weszlem(%d)\n", this->id);
	}
}



void Librarian::readAnswer(int id, int numbOfReaders, int answer){ 
	//printf("Metoda ReadAnswer()\n");
	if (answer == 100){ // 100 - kod dla odpowiedzi "agree"
		this->priorities[id] = 2; //2 - wartosc dla odpowiedzi "agree"
	}
	else if (answer == 200 && numbOfReaders > this->customersCount){
		this->priorities[id] = 0; // 0 - brak zezwolenia (przegrana walka)
	}
	else if (answer == 200 && numbOfReaders < this->customersCount){
		this->priorities[id] = 1; //1 - zezwolenie (wygrana walka)
	}
	else if (answer == 200 && numbOfReaders == this->customersCount && this->id > id){
		this->priorities[id] = 1;
	}
	else{
		this->priorities[id] = 0;
	}
}



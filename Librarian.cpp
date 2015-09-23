#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <list>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include "MPC.cpp"
#include "Technician.cpp"
#define ROOT 0
#define MSG_TAG 100
#define NUMB_OF_MPC 3

using namespace std;

class Librarian {
 
 private:
 	int id;
	int size;
	int customersCount;
	int msg[3];
	list<int> processesIds;
	int priorities[]; //[0] brak zgody [1] wygrana walka [2] zezwolenie
    MPC mpcArray[NUMB_OF_MPC];
	bool czyMogeWejsc;
	bool active; // czy ubiegam się o dostęp do MPC
	vector<int> activeLibrarians;
	int tempState[2]; //tymczasowa zmienna okreslajaca czy proces wysylajacy wiadomosc jest aktywny tempState[0]-id procesu, temsState[1]-stan
 public:
	Librarian(int id, int size);	
	void sendRequests();
	void canEnter();
	void waitForAnswears();
	void readAnswer(int id, int numOfReaders, int answer);
	void accessMPC();
	int getCustomersCount();
	bool isActive();
	void sendProcessState();
	void notifyAboutActivity();
	void gatherActiveProcesses();
};

int Librarian::getCustomersCount() {
	return this->customersCount;
}

bool Librarian::isActive() {
	return this->active;
}

Librarian::Librarian(int id, int size){
	srand(time(0));
	this->id = id;
	this->size = size;
	this->customersCount = rand()%(id+1) + 7;
	for(int i  = 0; i < size; i++) {
		 this->priorities[i] = 0;
		 activeLibrarians.push_back(0);
		 //this->activeLibrarians[i] = 0; //początkowo wszyscy jako nieaktywni
	}
	//activeLibrarians.reserve(size);
	for(int i = 0; i < NUMB_OF_MPC; i++) {
		this->mpcArray[i].id = i;
	}
	for(int i = 0; i < 2; i++){
		tempState[i] = 0;
	}
	// this->priorities[];
	this->priorities[id] = 1; //sam sobie zezwalam
	this->msg[0] = id;
	this->msg[1] = this->customersCount;
	this->czyMogeWejsc = false;
	this->active = rand()%(id+1);
}

void Librarian::notifyAboutActivity() {
	if(this->active) {
		this->activeLibrarians.at(this->id) = 1;
	}
	else{
		this->activeLibrarians.at(this->id) = 0;
	}
	this->tempState[0] = this->id;
	for(int i = 0; i < this->size; i++) {
		if(i != this->id) {
			if(this->active) {
				this->tempState[1] = 1;

				MPI_Send(tempState, 2, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
			}
			else{
				this->tempState[1] = 0;
				MPI_Send(tempState, 2, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
			}
		}
		
	}
}

void Librarian::gatherActiveProcesses() {
	for(int i = 0; i < (size-1); i++) {
			MPI_Status status;
			MPI_Recv(tempState, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			this->activeLibrarians.at(tempState[0]) = tempState[1];
	}
	printf("Active librarians array dla (%d):", this->id);

	for(int i = 0 ; i < size; i++) {
		printf(" %d ", this->activeLibrarians[i]);
	}
	printf("\n");
}


void Librarian::sendRequests() {
	//rozsyłanie requestów
	//if(this->active) {
		for (int i = 0; i<this->size; i++){
			if (i != this->id){
				this->msg[2] = 200;
				MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
			}
		}
	//}
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



void Librarian::readAnswer(int id, int numbOfReaders, int answer){ 
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


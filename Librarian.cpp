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
//#include <thread>
#define ROOT 0
#define MSG_TAG 100
#define NUMB_OF_MPC 3
#define MSG_ACTIVE_TAG 400
#define SELECT_MPC_TAG 500
#define MSG_RECENTLY_TAG 600
#define UPDATE_MPC_TAG 700

using namespace std;

class Librarian {
 
 private:
 	int id;
	int size;
	int customersCount;
	int msg[3];
	list<int> processesIds;
	int*  priorities; //[0] brak zgody [1] wygrana walka [2] zezwolenie
    //vector<MPC*> mpcArray;
    vector<MPC> mpcVector;
    MPC mpcArray[NUMB_OF_MPC];
	bool czyMogeWejsc;
	bool active; // czy ubiegam się o dostęp do MPC
	vector<int> activeLibrarians;
	int tempState[2]; //tymczasowa zmienna okreslajaca czy proces wysylajacy wiadomosc jest aktywny tempState[0]-id procesu, temsState[1]-stan
	bool engaged; //poczatkowo zeden librarian nie angazuje zadnego MPC
	bool recentlyEngaged;
	int* mpcAccess; // tablica w której okreslane jest czy jakis proces w danej iteracji dostal sie do MPC

	//vector<MPC>::iterator iterator;
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
	bool checkFreeMPC();
	void selectMPC();
	void updateMPCArray();
	void updateMPC();
	bool isEngaged();
	void notifyAboutMPCAccess();
	void gatherMPCAccess();
	void initializeMPCAccess();
	bool checkMPCAccessArray();
	void waitForMPCUpdate();
};

int Librarian::getCustomersCount() {
	return this->customersCount;
}

bool Librarian::isActive() {
	return this->active;
}

bool Librarian::isEngaged() {
	return this->engaged;
}

Librarian::Librarian(int id, int size){
	srand(time(0));
	this->id = id;
	this->size = size;
	this->priorities[this->size];
	this->customersCount = rand()%(id+1) + 7;
	this->engaged = false;//poczatkowo nie angazuje zadnego MPC
	priorities = new int[size];
	mpcAccess = new int[size];
	for(int i  = 0; i < size; i++) {
		 this->priorities[i] = 0;
		 activeLibrarians.push_back(0);
		 this->mpcAccess[i] = 0;
		 //this->activeLibrarians[i] = 0; //początkowo wszyscy jako nieaktywni
	}
	//activeLibrarians.reserve(size);

	for(int i = 0 ; i < NUMB_OF_MPC; i++) {
		mpcArray[i] = MPC(i, true, 0);
	}

	for(int i = 0 ; i < NUMB_OF_MPC; i++) {
		mpcVector.push_back(MPC(i, true, 0));
	}

	for(int i = 0; i < 2; i++){
		tempState[i] = 0;
	}

	this->msg[0] = id;
	this->msg[1] = this->customersCount;
	this->czyMogeWejsc = false;
	this->recentlyEngaged = false; //potrzebne zeby wyslac update MPC array do wszystkich oprócz siebie gdy zabiera mpc-ka
	this->active = rand()%(id+1);
	if(this->active) {
		this->priorities[id] = 1; //sam sobie zezwalam
	}
	else {
		this->priorities[id] = 0; //nie zezwalam sobie
	}
}

void Librarian::initializeMPCAccess() {
	for(int i = 0; i < this->size; i++) {
		mpcAccess[i] = 0;
	}
}

bool Librarian::checkMPCAccessArray() {
	for(int i = 0; i < this->size; i++) {
		if(mpcAccess[i] == 0){
			return false;
		}
	}
	return true;
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
				MPI_Send(tempState, 2, MPI_INT, i, MSG_ACTIVE_TAG, MPI_COMM_WORLD);
			}
			else{
				this->tempState[1] = 0;
				MPI_Send(tempState, 2, MPI_INT, i, MSG_ACTIVE_TAG, MPI_COMM_WORLD);
			}
		}
		
	}
}

void Librarian::gatherActiveProcesses() {
	//cout<<"Gather MPC Array size:"<<mpcArray.size()<<endl;
	for(int i = 0; i < (size-1); i++) {
			MPI_Status status;
			MPI_Recv(tempState, 2, MPI_INT, MPI_ANY_SOURCE, MSG_ACTIVE_TAG, MPI_COMM_WORLD, &status);
			this->activeLibrarians.at(tempState[0]) = tempState[1];
	}
}


void Librarian::canEnter() {
	//sprawdzenie tablicy priorytetów
	if(this->engaged || !this->active) {
			this->priorities[this->id] = 0;
		}
	else{
			this->priorities[this->id] = 1;
	}
	
	this->czyMogeWejsc = true;
	for (int i = 0; i < this->size; i++){
		if (this->priorities[i] == 0){
			this->czyMogeWejsc = false;
			break;
		}
	}
}

bool Librarian::checkFreeMPC() {
	//sprawdza czy istnieje wolny MPC
	//mpcVector.resize(3);
	bool free;
	for(int i = 0; i < NUMB_OF_MPC; i++) {
		if(!mpcArray[i].isFree()) {
			free = false;
		}
		else {
			return true;
		}
	} 
	return free;
}

void Librarian::notifyAboutMPCAccess() {
	int msg[1];
	msg[0] = this->id;
	msg[1] = 0;
	if(this->recentlyEngaged) {
			msg[1] = 1;
	}
	for(int i = 0; i < this->size; i++) {
		if(i != this->id) {
			MPI_Send(msg, 2, MPI_INT, i, MSG_RECENTLY_TAG, MPI_COMM_WORLD);
		}
	}
}

void Librarian::gatherMPCAccess() {
	int msg[1];
	for(int i = 0; i < (size-1); i++) {
			MPI_Status status;
			MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, MSG_RECENTLY_TAG, MPI_COMM_WORLD, &status);

			for(int i = 0 ; i < size; i++) {
				if(msg[0] == i) {
					mpcAccess[i] = msg[1];
				}
			}
	}
}


void Librarian::waitForMPCUpdate() {
	MPI_Status status;
	int mpc[3];
	//while(1) {
		MPI_Recv(mpc, 4, MPI_INT, MPI_ANY_SOURCE, UPDATE_MPC_TAG, MPI_COMM_WORLD, &status);
		for(int i = 0; i < NUMB_OF_MPC; i++) {
				if(mpcArray[i].getId() == mpc[0]) {
					if(mpc[1] == 1) {
						mpcArray[i].setFree(false);
						mpcArray[i].addServedCustomers(mpc[2]);
						mpcArray[i].setCurrentLibrarianId(mpc[3]);
					}
					else if(mpc[1] == 0){
						mpcArray[i].setFree(true);
					}
				}
		}
	//}
}


// metoda dla wątkuppp
void Librarian::updateMPCArray() {
	MPI_Status status;
	int mpc[3];

	if(!this->recentlyEngaged){
		if(this->checkMPCAccessArray()) {
			MPI_Recv(mpc, 4, MPI_INT, MPI_ANY_SOURCE, SELECT_MPC_TAG, MPI_COMM_WORLD, &status);
			for(int i = 0; i < NUMB_OF_MPC; i++) {
				if(mpcArray[i].getId() == mpc[0]) {
					if(mpc[1] == 1) {
						mpcArray[i].setFree(false);
						mpcArray[i].addServedCustomers(mpc[2]);
						mpcArray[i].setCurrentLibrarianId(mpc[3]);
					}
					else if(mpc[1] == 0){
						mpcArray[i].setFree(true);
					}
				}
			}
		}
	}	
	this->recentlyEngaged = false;
}


void Librarian::selectMPC() {
//zabieram MPC i powiadamiam innych	
	for(int i = 0 ; i < NUMB_OF_MPC; i++) {
		if(mpcArray[i].isFree()) {
			mpcArray[i].setFree(false);
			mpcArray[i].setCurrentLibrarianId(this->id);
			mpcArray[i].addServedCustomers(this->customersCount);
			this->engaged = true; //proces angazuje MPC
			this->recentlyEngaged = true;
			this->mpcAccess[this->id] = 1;
			for(int j = 0; j < this->size; j++) {
				if(j != this->id) {
					int mpc[3]; //mpc[0] - id MPC, mpc[1] - jesli 1 to zajety, jesli 0 to zwolniony, mpc[2] - ilosc customers
					mpc[0] = mpcArray[i].getId();
					mpc[1] = 1;
					mpc[2] = this->customersCount;
					mpc[3] = this->id;
					MPI_Send(mpc, 4, MPI_INT, j, SELECT_MPC_TAG, MPI_COMM_WORLD);
				}
			}
			break;
		}
	}
}


void Librarian::accessMPC() {
	//dostêp do MPC
	if (this->czyMogeWejsc){
		bool freeMPC = checkFreeMPC();
		if(freeMPC) {
			printf("Są wolne\n");
			selectMPC();
			printf("Wszedlem(%d)--------------------------------------\n", this->id);
		}
	}else{
		printf("Nie wszedlem(%d)\n", this->id);
	}
	printf("(%d): ", this->id);
	for(int i=0; i<this->size; i++)
	{
		printf(" %d", this->priorities[i]);
	}
	cout<<endl;
	notifyAboutMPCAccess();
	gatherMPCAccess();
}


void Librarian::waitForAnswears() {
	for (int i = 0; i < (this->size-1); i++){ //oczekuje na size-1 odpowiedzi
		MPI_Status status;
		MPI_Recv(msg, 3, MPI_INT, MPI_ANY_SOURCE, MSG_TAG, MPI_COMM_WORLD, &status);
		this->readAnswer(this->msg[0], this->msg[1], this->msg[2]);
	}
}

void Librarian::sendRequests() {
	//rozsyłanie requestów
	initializeMPCAccess();
		for (int i = 0; i<this->size; i++){
			if (i != this->id){
				if(this->active && !this->engaged) { // jesli jest aktywny i niezaangazowany
					msg[0] = this->id;
					msg[1] = this->customersCount;
					this->msg[2] = 200;
					MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);	
				}
				else if(this->active && this->engaged) {
					msg[0] = this->id;
					msg[1] = this->customersCount;
					this->msg[2] = 400; //kolejna iteracja, jesli jest angazujacy to wysyła komunikat 300
					MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
				}
				else {
					msg[0] = this->id;
					msg[1] = this->customersCount;
					this->msg[2] = 100;
					MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
				}
				
			}
		}
}

void Librarian::readAnswer(int id, int numbOfReaders, int answer){ 
	//printf("(%d | %d)Odpowiedz od: %d | %d | %d\n", this->id, this->customersCount, id, numbOfReaders, answer);
	if (answer == 100){ // 100 - kod dla odpowiedzi "agree"
		this->priorities[id] = 2; //2 - wartosc dla odpowiedzi "agree"
		return;
	}
	if(answer == 400) {
		this->priorities[id] = 4;
		return;
	}
	if(answer == 200 && (numbOfReaders > this->customersCount)){
		this->priorities[id] = 1; // 0 - brak zezwolenia (przegrana walka)
		return;
	}
	if(answer == 200 && (numbOfReaders < this->customersCount)){
		this->priorities[id] = 0; //1 - zezwolenie (wygrana walka)
		return;
	}
	else if(answer == 200 && numbOfReaders == this->customersCount && id >= this->id){
		this->priorities[id] = 1;
		return;
	}
}


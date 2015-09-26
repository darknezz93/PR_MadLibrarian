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
#define NUMB_OF_MPC 2
#define MSG_ACTIVE_TAG 400
#define SELECT_MPC_TAG 500

using namespace std;

class Librarian {
 
 private:
 	int id;
	int size;
	int customersCount;
	int msg[3];
	list<int> processesIds;
	int priorities[]; //[0] brak zgody [1] wygrana walka [2] zezwolenie
    //vector<MPC*> mpcArray;
    vector<MPC> mpcVector;
    MPC mpcArray[NUMB_OF_MPC];
	bool czyMogeWejsc;
	bool active; // czy ubiegam się o dostęp do MPC
	vector<int> activeLibrarians;
	int tempState[2]; //tymczasowa zmienna okreslajaca czy proces wysylajacy wiadomosc jest aktywny tempState[0]-id procesu, temsState[1]-stan
	bool engaged; //poczatkowo zeden librarian nie angazuje zadnego MPC
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
	this->customersCount = rand()%(id+1) + 7;
	this->engaged = false;//poczatkowo nie angazuje zadnego MPC
	for(int i  = 0; i < size; i++) {
		 this->priorities[i] = 0;
		 activeLibrarians.push_back(0);
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

	//cout<<"mpcArray size: "<<mpcArray.size()<<endl;

	this->msg[0] = id;
	this->msg[1] = this->customersCount;
	this->czyMogeWejsc = false;
	this->active = rand()%(id+1);
	if(this->active) {
		this->priorities[id] = 1; //sam sobie zezwalam
	}
	else {
		this->priorities[id] = 0; //nie zezwalam sobie
	}
}

void Librarian::notifyAboutActivity() {

	//printf("%d", mpcArray.at(0).id);
	//cout<<"Notify MPC Array size:"<<mpcArray.size()<<endl;
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
	//printf("Active librarians array dla (%d):", this->id);

	for(int i = 0 ; i < size; i++) {
		printf(" %d ", this->activeLibrarians[i]);
	}
	printf("\n");
}


void Librarian::sendRequests() {
	//rozsyłanie requestów
	//cout<<"sendRequests MPC Array size:"<<mpcArray.size()<<endl;
		for (int i = 0; i<this->size; i++){
			if (i != this->id){
				if(this->active && !this->engaged) { // jesli jest aktywny i niezaangazowany
					this->msg[2] = 200;
					MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);	
				}
				else {
					this->msg[2] = 100;
					MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
				}
				
			}
		}
}

void Librarian::canEnter() {
	//sprawdzenie tablicy priorytetów
	this->czyMogeWejsc = true;
	for (int i = 0; i < this->size; i++){
		if (this->priorities[i] == 0){
			this->czyMogeWejsc = false;
			break;
		}
		//cout<<"Can enter MPC Array size:"<<mpcArray.size()<<endl;
	}
}

void Librarian::waitForAnswears() {

	//cout<<"waitForAnswears MPC Array size:"<<mpcArray.size()<<endl;
	for (int i = 1; i < this->size; i++){ //oczekuje na size-1 odpowiedzi
		MPI_Status status;
		MPI_Recv(msg, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		this->readAnswer(this->msg[0], this->msg[1], this->msg[2]);
	}
}

bool Librarian::checkFreeMPC() {
	//sprawdza czy istnieje wolny MPC
	mpcVector.resize(3);
	//cout<<"checkFreeMPC MPC Array size:"<<mpcVector.size()<<endl;
	for(int i = 0; i < 1; i++) {
		if(!mpcArray[i].isFree()) {
			return false;
		}
	} 
	printf("PRAWDA\n");
	return true;
}

void Librarian::updateMPCArray() {
	MPI_Status status;
	int mpc[1];
	mpc[0] = 4545;

	if(!this->engaged){
	//printf("Chce odebrac update %d\n", this->id);	
	MPI_Recv(mpc, 2, MPI_INT, MPI_ANY_SOURCE, SELECT_MPC_TAG, MPI_COMM_WORLD, &status);
	//printf("ODEBRAŁEM %d, mpc[0]= %d, mpc[1] = %d\n", this->id, mpc[0], mpc[1]);
	for(int i = 0; i < NUMB_OF_MPC; i++) {
		if(mpcArray[i].getId() == mpc[0]) {
			if(mpc[1] == 1) {
				mpcArray[i].setFree(false);
			}
			else if(mpc[1] == 0){
				mpcArray[i].setFree(true);
			}
		}
	}
	for(int k = 0; k < NUMB_OF_MPC; k++) {
		printf("MPC ARRAY PO UPDATE: id-%d, free-%d, customers-%d\n", mpcArray[k].getId(), mpcArray[k].isFree(), mpcArray[k].getServedCustomers());
	}

	}	


}


void Librarian::selectMPC() {
//zabieram MPC i powiadamiam innych	
	for(int i = 0 ; i < NUMB_OF_MPC; i++) {
		if(mpcArray[i].isFree()) {
			mpcArray[i].setFree(false);
			this->engaged = true; //proces angazuje MPC
			for(int j = 0; j < this->size; j++) {
				if(j != this->id) {
					int mpc[1]; //mpc[0] - id MPC, mpc[1] - jesli 1 to zajety, jesli 0 to zwolniony
					mpc[0] = mpcArray[i].getId();
					mpc[1] = 1;
					printf("Wysyłam %d, mpc[0] = %d, mpc[1] = %d\n", j, mpc[0], mpc[1]);
					MPI_Send(mpc, 2, MPI_INT, j, SELECT_MPC_TAG, MPI_COMM_WORLD);
				}
			}
		}
		break;
	}
}


void Librarian::accessMPC() {
	//dostêp do MPC

	if (this->czyMogeWejsc){
		bool freeMPC = checkFreeMPC();
		if(freeMPC) {
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
}



void Librarian::readAnswer(int id, int numbOfReaders, int answer){ 
	//printf("Metoda ReadAnswer()\n");
	//printf("(%d | %d)Odpowiedz od: %d | %d | %d\n", this->id, this->customersCount, id, numbOfReaders, answer);
	if (answer == 100){ // 100 - kod dla odpowiedzi "agree"
		this->priorities[id] = 2; //2 - wartosc dla odpowiedzi "agree"
		return;
	}
	if (answer == 200 && numbOfReaders > this->customersCount){
		this->priorities[id] = 1; // 0 - brak zezwolenia (przegrana walka)
		return;
	}
	if (answer == 200 && numbOfReaders < this->customersCount){
		this->priorities[id] = 0; //1 - zezwolenie (wygrana walka)
		return;
	}
	if (numbOfReaders == this->customersCount && id < this->id)
	{
		this->priorities[id] = 0;
		return;
	}
	if(numbOfReaders == this->customersCount && id >= this->id){
		this->priorities[id] = 1;
		return;
	}
}


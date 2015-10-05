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
#define NUMB_OF_TECHNICIANS 1
#define LIMIT 10
#define MSG_ACTIVE_TAG 400
#define SELECT_MPC_TAG 500
#define MSG_RECENTLY_TAG 600
#define UPDATE_MPC_TAG 700
#define MSG_TECH_ACCESS_TAG 800
#define MSG_UPDATE_TECH_TAG 900

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
    Technician technicianArray[NUMB_OF_TECHNICIANS]; //tablica serwisantów
	bool czyMogeWejsc;
	bool active; // czy ubiegam się o dostęp do MPC
	vector<int> activeLibrarians;
	int tempState[2]; //tymczasowa zmienna okreslajaca czy proces wysylajacy wiadomosc jest aktywny tempState[0]-id procesu, temsState[1]-stan
	bool engaged; //poczatkowo zeden librarian nie angazuje zadnego MPC
	bool recentlyEngaged;
	bool recentlyTechnician;
	int* mpcAccess; // tablica w której okreslane jest czy jakis proces w danej iteracji dostal sie do MPC
	int* technicianAccess; //tablica w kórej okreslane jest czy jakis proces w danej iteracji dostał sie do serwisanta
	bool waitingForTechnician; //okresla czy Librarian ubiega się o serwisanta
	bool usingTechnician; //okresla czy Librarian obecnie uzywa jakiegos serwisanta

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
	void releaseMPC();
	void randomActivity();
	bool isMachingMPC();
	bool checkFreeTechnicians();
	void useTechnician();
	void notifyAboutTechnicianAccess();
	void gatherTechnicianAccess();
	void updateTechnicianArray();
	bool checkTechnicianAccessArray();
	void sendTechnicianArray();
	void initializeTechnicianAccessArray();
	void releaseTechnicianAndMPC();	
	void updateTechnicianAndMPC();
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
	this->waitingForTechnician = false;
	this->usingTechnician = false;
	priorities = new int[size];
	mpcAccess = new int[size];
	technicianAccess = new int[size];

	for(int i  = 0; i < size; i++) {
		 this->priorities[i] = 0;
		 activeLibrarians.push_back(0);
		 this->mpcAccess[i] = 0;
		 this->technicianAccess[i] = 0;
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

	for(int i = 0 ;i < NUMB_OF_TECHNICIANS; i++) {
		technicianArray[i] = Technician(i, true, -1);
	}

	this->msg[0] = id;
	this->msg[1] = this->customersCount;
	this->czyMogeWejsc = false;
	this->recentlyEngaged = false; //potrzebne zeby wyslac update MPC array do wszystkich oprócz siebie gdy zabiera mpc-ka
	this->recentlyTechnician = false;
	this->active = rand()%(2);
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
		if(mpcAccess[i] == 1){
			return true;
		}
	}
	return false;
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

	if(this->engaged && this->waitingForTechnician) {
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

			for(int j = 0 ; j < size; j++) {
				if(msg[0] == j) {
					mpcAccess[j] = msg[1];
				}
			}
	}
}


// metoda dla wątkuppp
void Librarian::updateMPCArray() {
	//mpc[0] - id MPC, mpc[1] - jesli 1 to zajety, jesli 0 to zwolniony, mpc[2] - ilosc customers, mpc[3] - currentLibrarianId
	MPI_Status status;
	int mpc[3];


	for(int i = 0; i < (this->size-1); i++) {
		MPI_Recv(mpc, 4, MPI_INT, MPI_ANY_SOURCE, SELECT_MPC_TAG, MPI_COMM_WORLD, &status);
		if(mpc[0] != -2) {
			for(int j = 0; j < NUMB_OF_MPC; j++) {
				if(mpcArray[j].getId() == mpc[0]) {
					if(mpc[1] == 0) {
						mpcArray[j].setFree(true);
					}
					else if(mpc[1] == 1) {
						mpcArray[j].setFree(false);
					}
					mpcArray[j].setServedCustomers(mpc[2]);
					mpcArray[j].setCurrentLibrarianId(mpc[3]);
				}
			}
		}
	}

	/*for(int d = 0; d < NUMB_OF_MPC; d++) {
		printf("PRZED [%d]  Id- %d, free-%d, served-%d, librarian-%d\n",this->id, this->mpcArray[d].getId(), this->mpcArray[d].isFree(), this->mpcArray[d].getServedCustomers(), this->mpcArray[d].getCurrentLibrarianId());
	}
	printf("\n");
`*/


	/*if(!this->recentlyEngaged){
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
	} */	

}

bool Librarian::isMachingMPC() {
//sprawdza czy w zbiorze MPC jest mpc, który jest w stanie obsluzyc librariana	
	for(int i = 0; i < NUMB_OF_MPC; i++) {
		if(mpcArray[i].isFree()) {
			if((mpcArray[i].getServedCustomers() + this->customersCount) < LIMIT) {
				return true;
			}
		}
	}
	return false;
}


void Librarian::releaseMPC() {
	int mpc[3]; //mpc[0] - id MPC, mpc[1] - jesli 1 to zajety, jesli 0 to zwolniony, mpc[2] - ilosc customers, mpc[3] - currentLibrarianId
	int currentLibrarianId = -1;

	/*
	Mpc update jest tylko wtedy gdy nie bierzemy serwisanta !!!
	Gdy bierzemy serwisanta to update mpc i update serwisanta pojdzie w ReleaseTechnician
	*/
	if(this->engaged && !this->waitingForTechnician && !this->usingTechnician) {
		for(int i = 0; i < NUMB_OF_MPC; i++) {
			if(mpcArray[i].getCurrentLibrarianId() == this->id) {
				//srand(time(0));
				bool releaseMPC = rand()%(2);
				if(releaseMPC) {
					this->engaged = false;
					mpcArray[i].setFree(true);
					mpcArray[i].setCurrentLibrarianId(-1);
					//printf("PRZED WYLOSOWANIEM: %d\n", this->customersCount);
					this->customersCount = rand()%(id+1) + 10;
					mpc[1] = 0;
					printf("ZWALNIAM MPC: %d\n", this->id);
					printf("WYLOSOWAŁEM: %d\n", this->customersCount);
				}
				else {
					mpc[1] = 1;
					currentLibrarianId = this->id;
				}
				for(int j = 0; j < this->size; j++) {
					if(j != this->id) {
						mpc[0] = mpcArray[i].getId();
						//mpc[1] = 1;
						mpc[2] = mpcArray[i].getServedCustomers();	
					
						mpc[3] = mpcArray[i].getCurrentLibrarianId();
						MPI_Send(mpc, 4, MPI_INT, j, SELECT_MPC_TAG, MPI_COMM_WORLD);
					}
			  	}
			}
 		}
	}
	else {
		mpc[0] = -2;
		mpc[1] = -2;
		mpc[2] = -2;
		mpc[3] = -2;
		for(int j = 0; j < this->size; j++) {
			if(j != this->id) {
				MPI_Send(mpc, 4, MPI_INT, j, SELECT_MPC_TAG, MPI_COMM_WORLD);
			}
		}
	}
}

void Librarian::releaseTechnicianAndMPC() {
	MPI_Status status;
	int msg[7];
	/*	msg[0] - id wiadomosci
		msg[1] - id serwisanta
		msg[2] - free serwisanta
		msg[3] - currentLibrarianId serwisanta
		msg[4] - id mpc
		msg[5] - free mpc
		msg[6] - currentLibrarianId mpc
		msg[7] - servedCustomers mpc
	*/
	if(this->usingTechnician) {
		for(int i = 0; i < NUMB_OF_TECHNICIANS; i++) {
			if(this->technicianArray[i].getCurrentLibrarianId() == this->id) {
				//srand(time(0));
				bool releaseTechnician = rand()%(2);

				if(releaseTechnician && recentlyTechnician) {
					//Zwalniam i w tej iteracji brałem serwisanta
					printf("Zwalniam serwisanta (%d)\n", this->id);
					//zwalniam serwisanta
					this->technicianArray[i].setFree(true);
					this->technicianArray[i].setCurrentLibrarianId(-1);
					this->usingTechnician = false;
					this->engaged = false;
					for(int j = 0; j < NUMB_OF_MPC; j++) {
						//zwalniam mpc
						if(this->mpcArray[j].getCurrentLibrarianId() == this->id) {
							this->mpcArray[j].setFree(true);
							this->mpcArray[j].setCurrentLibrarianId(-1);
							this->mpcArray[j].setServedCustomers(0);
							//rozsyłam do wszystkich
							for(int k = 0; k < size; k++) {
								if(k != this->id) {
									msg[0] = 0;
									msg[1] = this->technicianArray[i].getId();
									msg[2] = 1;
									msg[3] = this->technicianArray[i].getCurrentLibrarianId();
									msg[4] = this->mpcArray[j].getId();
									msg[5] = 1;
									msg[6] = this->mpcArray[j].getCurrentLibrarianId();
									msg[7] = this->mpcArray[j].getServedCustomers();
									//printf("A\n");
									MPI_Send(msg, 8, MPI_INT, k, MSG_UPDATE_TECH_TAG, MPI_COMM_WORLD);
								}
							}
						}

					}
				}
				else if(releaseTechnician && !recentlyTechnician) {
				//zwalniam ale brałem serwisanta w poprzednich iteracjach
					printf("Zwalniam serwisanta (%d)", this->id);
					this->technicianArray[i].setFree(true);
					this->technicianArray[i].setCurrentLibrarianId(-1);
					this->usingTechnician = false;
					this->engaged = false;
					for(int j = 0; j < NUMB_OF_MPC; j++) {
						if(this->mpcArray[j].getCurrentLibrarianId() == this->id) {
							//zwalniam mpc
							this->mpcArray[j].setFree(true);
							this->mpcArray[j].setCurrentLibrarianId(-1);
							this->mpcArray[j].setServedCustomers(0);
							//rozsyłam do wszystkich
							for(int k = 0; k < size; k++) {
								if(k != this->id) {
									msg[0] = 1;
									msg[1] = this->technicianArray[i].getId();
									msg[2] = 1; // wolny serwisant
									msg[3] = this->technicianArray[i].getCurrentLibrarianId();
									msg[4] = this->mpcArray[j].getId();
									msg[5] = 1; // wolny mpc
									msg[6] = this->mpcArray[j].getCurrentLibrarianId();
									msg[7] = this->mpcArray[j].getServedCustomers();
									//printf("A\n");
									MPI_Send(msg, 8, MPI_INT, k, MSG_UPDATE_TECH_TAG, MPI_COMM_WORLD);
								}
							}
						}

					}
					
				}
				else if(!releaseTechnician && recentlyTechnician) {
				//nie zwalniam ale wzielem serwisanta w tej iteracji
					this->technicianArray[i].setFree(false);
					this->technicianArray[i].setCurrentLibrarianId(this->id);
					for(int j = 0; j < NUMB_OF_MPC; j++) {
						//zwalniam mpc
						if(this->mpcArray[j].getCurrentLibrarianId() == this->id) {
							//rozsyłam do wszystkich
							for(int k = 0; k < size; k++) {
								if(k != this->id) {
									msg[0] = 2;
									msg[1] = this->technicianArray[i].getId();
									msg[2] = 0; // zajety serwisant
									msg[3] = this->technicianArray[i].getCurrentLibrarianId();
									msg[4] = this->mpcArray[j].getId();
									msg[5] = 0; // zajety mpc
									msg[6] = this->mpcArray[j].getCurrentLibrarianId();
									msg[7] = this->mpcArray[j].getServedCustomers();
									//printf("A\n");
									MPI_Send(msg, 8, MPI_INT, k, MSG_UPDATE_TECH_TAG, MPI_COMM_WORLD);
								}
							}
						}

					}	
				}
				else if(!releaseTechnician && !recentlyTechnician) {
					for(int a = 0; a < this->size; a++) {
						if(a != this->id) {
							msg[0] = 3;
							msg[1] = -2;
							msg[2] = -2;
							msg[3] = -2;
							msg[4] = -2;
							msg[5] = -2;
							msg[6] = -2;
							msg[7] = -2;
							//printf("B\n");
							MPI_Send(msg, 8, MPI_INT, a, MSG_UPDATE_TECH_TAG, MPI_COMM_WORLD);
						}
					}
				}
				else {
					for(int a = 0; a < this->size; a++) {
						if(a != this->id) {
							msg[0] = 3;
							msg[1] = -2;
							msg[2] = -2;
							msg[3] = -2;
							msg[4] = -2;
							msg[5] = -2;
							msg[6] = -2;
							msg[7] = -2;
							//printf("B\n");
							MPI_Send(msg, 8, MPI_INT, a, MSG_UPDATE_TECH_TAG, MPI_COMM_WORLD);
						}
					}
				}
			}

		}
	}
	else if(this->waitingForTechnician && this->recentlyEngaged) {
		/*MOZE WYSYLAC NAWET JAK SIE NIE UZYWA TECHNICIANA I WTEDY BY SIE PRZY ODBIORZE NIC NIE UPDATOWAŁO ? */
		for(int j = 0; j < NUMB_OF_MPC; j++) {
		//zwalniam mpc
			if(this->mpcArray[j].getCurrentLibrarianId() == this->id) {
			//rozsyłam do wszystkich
				for(int k = 0; k < size; k++) {
					if(k != this->id) {
						//update tylko dla mpcka
						msg[0] = 4;
						msg[1] = -2;
						msg[2] = -2;
						msg[3] = -2;
						msg[4] = this->mpcArray[j].getId();
						msg[5] = 0; // zajety mpc
						msg[6] = this->mpcArray[j].getCurrentLibrarianId();
						msg[7] = this->mpcArray[j].getServedCustomers();
						MPI_Send(msg, 8, MPI_INT, k, MSG_UPDATE_TECH_TAG, MPI_COMM_WORLD);
						//printf("WYSYŁAM 4\n");
					}
				}
			}

		}
	}
	else {
		for(int b = 0; b < this->size; b++) {
			if(b != this->id) {
				msg[0] = 3;
				msg[1] = -2;
				msg[2] = -2;
				msg[3] = -2;
				msg[4] = -2;
				msg[5] = -2;
				msg[6] = -2;
				msg[7] = -2;
				//printf("WYSYŁAM -3 (%d)\n", this->id);
				//printf("C\n");
				MPI_Send(msg, 8, MPI_INT, b, MSG_UPDATE_TECH_TAG, MPI_COMM_WORLD);				
			}
		}		
	}

}




void Librarian::updateTechnicianAndMPC() {
	/*	msg[0] - id wiadomosci
		msg[1] - id serwisanta
		msg[2] - free serwisanta
		msg[3] - currentLibrarianId serwisanta
		msg[4] - id mpc
		msg[5] - free mpc
		msg[6] - currentLibrarianId mpc
		msg[7] - servedCustomers mpc
	*/
	//sleep(1);
	
	MPI_Status status;
	for (int i = 0; i < (this->size-1); i++){ //oczekuje na size-1 odpowiedzi
		int msg[7];
		//printf("CHCE ODEBRAC (%d)\n", this->id);
		MPI_Recv(msg, 8, MPI_INT, MPI_ANY_SOURCE, MSG_UPDATE_TECH_TAG, MPI_COMM_WORLD, &status);
		//printf("ODEBRAŁEM (%d)\n", this->id);
		if(msg[0] == 0 || msg[0] == 1) {
			//update techniciana
			//printf("B\n");
			for(int j = 0; j < NUMB_OF_TECHNICIANS; j++) {
				if(this->technicianArray[j].getId() == msg[1]) {
					this->technicianArray[j].setFree(true);
					this->technicianArray[j].setCurrentLibrarianId(msg[3]);
				}
			}
			for(int k = 0; k < NUMB_OF_MPC; k++) {
				if(this->mpcArray[k].getId() == msg[4]) {
					this->mpcArray[k].setFree(true);
					this->mpcArray[k].setCurrentLibrarianId(msg[6]);
					this->mpcArray[k].setServedCustomers(msg[7]);
				}
			}
		}
		else if(msg[0] == 2) {
			for(int j = 0; j < NUMB_OF_TECHNICIANS; j++) {
				if(this->technicianArray[j].getId() == msg[1]) {
					this->technicianArray[j].setFree(false);
					this->technicianArray[j].setCurrentLibrarianId(msg[3]);
				}
			}
			for(int k = 0; k < NUMB_OF_MPC; k++) {
				if(this->mpcArray[k].getId() == msg[4]) {
					this->mpcArray[k].setFree(false);
					this->mpcArray[k].setCurrentLibrarianId(msg[6]);
					this->mpcArray[k].setServedCustomers(msg[7]);
				}
			}			
		}
		else if(msg[0] == 4) {
			//printf("ODBIERAM 4\n");
			for(int j = 0; j < NUMB_OF_MPC; j++) {
				if(this->mpcArray[j].getId() == msg[4]) {
					this->mpcArray[j].setFree(false);
					this->mpcArray[j].setCurrentLibrarianId(msg[6]);
					this->mpcArray[j].setServedCustomers(msg[7]);
				}
			}
		}

	}

	/*for(int d = 0; d < NUMB_OF_TECHNICIANS; d++) {
		printf("SERWISANT [%d]  Id- %d, free-%d,librarian-%d\n",this->id, this->technicianArray[d].getId(), this->technicianArray[d].isFree(), this->technicianArray[d].getCurrentLibrarianId());
	}
	printf("\n"); */

	for(int d = 0; d < NUMB_OF_MPC; d++) {
		printf("[%d]  Id- %d, free-%d, served-%d, librarian-%d\n",this->id, this->mpcArray[d].getId(), this->mpcArray[d].isFree(), this->mpcArray[d].getServedCustomers(), this->mpcArray[d].getCurrentLibrarianId());
	}
	printf("\n");
	//sleep(1);

	//printf("WYSZEDŁEM Z RECEVE: (%d)\n", this->id);
	this->recentlyEngaged = false;
	this->recentlyTechnician = false;
}




void Librarian::selectMPC() {
//zabieram MPC i powiadamiam innych	
	for(int i = 0 ; i < NUMB_OF_MPC; i++) {
		if(mpcArray[i].isFree()) {
			if((mpcArray[i].getServedCustomers() + this->customersCount) < LIMIT) {
				mpcArray[i].setFree(false);
				mpcArray[i].setCurrentLibrarianId(this->id);
				mpcArray[i].addServedCustomers(this->customersCount);
				this->engaged = true; //proces angazuje MPC
				this->recentlyEngaged = true;
				this->mpcAccess[this->id] = 1;
				break;

			}
			else {
				if(!this->isMachingMPC()) {
					//bierzesz MPC i udajesz sie do serwisanta
					mpcArray[i].setFree(false);
					mpcArray[i].addServedCustomers(this->customersCount);
					mpcArray[i].setCurrentLibrarianId(this->id);
					this->engaged = true;
					this->waitingForTechnician = true;
				    this->recentlyEngaged = true;
					this->mpcAccess[this->id] = 1;
					printf("BIORE mpc o id: (%d)", mpcArray[i].getId());
					break;
				}
			}			
		}
	}

}

void Librarian::accessMPC() {
	//dostêp do MPC
	if(this->czyMogeWejsc) {
		bool freeMPC = checkFreeMPC();
		bool freeTechnician = checkFreeTechnicians();
		if(freeMPC || this->waitingForTechnician) {
			//printf("Są wolne\n");
			if(!this->waitingForTechnician) {
				selectMPC();
			}
			if(!this->waitingForTechnician) {
				printf("Wszedlem(%d)--------------------------------------\n", this->id);
			}
			else {
				printf("Ubiegam sie o serwisanta(%d)\n", this->id);
				if(this->checkFreeTechnicians()) {
					this->useTechnician();
					printf("Dostałem serwisanta (%d)\n", this->id);
					printf("Wszedlem(%d)--------------------------------------\n", this->id);
				}
				else {
					printf("Nie ma wolnych serwisantow\n");
					printf("Czekam na serwisanta\n", this->id);
				}


			}

		}
		/*else if(this->waitingForTechnician && this->checkFreeTechnicians()) {
			this->useTechnician();
			printf("Dostałem sie do serwisanta (%d)", this->id);
		} */
	}else{
		printf("Nie wszedlem(%d)\n", this->id);
	}

	this->notifyAboutTechnicianAccess();
	this->gatherTechnicianAccess();

	//sleep(1);
	/*printf("Po updacie: (%d)", this->id);
				for(int i = 0; i < NUMB_OF_TECHNICIANS; i++) {
					printf("%d", technicianArray[i].isFree());
				}
				printf("\n"); */


	printf("(%d): ", this->id);
	for(int i=0; i<this->size; i++)
	{
		printf(" %d", this->priorities[i]);
	}
	printf("\n");

	this->notifyAboutMPCAccess();
	this->gatherMPCAccess();
}

void Librarian::useTechnician() {
	if(this->waitingForTechnician) {
		for(int i = 0; i < NUMB_OF_TECHNICIANS; i++) {
		if(technicianArray[i].isFree()) {
			technicianArray[i].setFree(false);
			technicianArray[i].setCurrentLibrarianId(this->id);
			this->usingTechnician = true;
			this->mpcAccess[this->id] = 1;
			this->recentlyTechnician = true;
			this->technicianAccess[this->id] = 1;
			this->waitingForTechnician = false;
			break;
		}
	}
  }	
}


void Librarian::notifyAboutTechnicianAccess() {
	int msg[1];
	msg[0] = this->id;
	msg[1] = 0; //wstepnie nie uzywa 
	if(this->usingTechnician) {
		msg[1] = 1;
	}
	for(int i = 0; i < this->size; i++) {
		if(i != this->id) {
			//printf("Notify (%d)\n", this->id);
			MPI_Send(msg, 2, MPI_INT, i, MSG_TECH_ACCESS_TAG, MPI_COMM_WORLD);
		}
	}
}

void Librarian::gatherTechnicianAccess() {
	int msg[1];
	for(int i = 0; i < (this->size-1); i++) {
			MPI_Status status;
			//printf("Gather (%d)\n", this->id);
			MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, MSG_TECH_ACCESS_TAG, MPI_COMM_WORLD, &status);

			for(int j = 0 ; j < size; j++) {
				if(msg[0] == j) {
					this->technicianAccess[j] = msg[1];
				}
			}
	}

}

bool Librarian::checkTechnicianAccessArray() {
	for(int i = 0; i < size; i++) {
		if(this->technicianAccess[i] == 1) {
			return true;
		}
	}
	return false;
}


bool Librarian::checkFreeTechnicians() {
	for(int i = 0; i < NUMB_OF_TECHNICIANS; i++) {
		if(technicianArray[i].isFree()) {
			return true;
		}
	}
	return false;
}


void Librarian::waitForAnswears() {
	for (int i = 0; i < (this->size-1); i++){ //oczekuje na size-1 odpowiedzi
		MPI_Status status;
		MPI_Recv(msg, 3, MPI_INT, MPI_ANY_SOURCE, MSG_TAG, MPI_COMM_WORLD, &status);
		this->readAnswer(this->msg[0], this->msg[1], this->msg[2]);
	}
}

void Librarian::initializeTechnicianAccessArray() {
	for(int i = 0; i < NUMB_OF_TECHNICIANS; i++) {
		this->technicianAccess[i] = 0;
	}
}

void Librarian::sendRequests() {
	//rozsyłanie 
	/*printf("Technicians: \n");
	for(int i = 0; i < NUMB_OF_TECHNICIANS; i++) {
		printf("id- %d,free- %d, currentLibrarianId- %d\n", technicianArray[i].getId(), technicianArray[i].isFree(), technicianArray[i].getCurrentLibrarianId());
	}*/
	initializeMPCAccess();
	initializeTechnicianAccessArray();

		for (int i = 0; i<this->size; i++){
			if (i != this->id){
				if(this->active && !this->engaged) { // jesli jest aktywny i niezaangazowany
					msg[0] = this->id;
					msg[1] = this->customersCount;
					this->msg[2] = 200;
					MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);	
				}
				else if(this->active && this->engaged && !this->waitingForTechnician) {
					msg[0] = this->id;
					msg[1] = this->customersCount;
					this->msg[2] = 400; //kolejna iteracja, jesli jest angazujacy to wysyła komunikat 400
					MPI_Send(msg, 3, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
				}
				else if(this->active && this->engaged && this->waitingForTechnician) {
					msg[0] = this->id;
					msg[1] = this->customersCount;
					this->msg[2] = 600;
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
		//printf("WYSŁAŁEM (%d)\n", this->id);
}

void Librarian::randomActivity() {
	if(!this->engaged) {
		if(!this->waitingForTechnician) {
			if(!this->usingTechnician) {
				this->active = rand()%(id+1);
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
	if(answer == 600 && (numbOfReaders >= this->customersCount) && this->waitingForTechnician) {
		this->priorities[id] = 1;
		return;
	}
	if(answer == 600 && (numbOfReaders < this->customersCount) && this->waitingForTechnician) {
		this->priorities[id] = 0;
		return;
	}
	else if(answer == 600 && !this->waitingForTechnician) {
		this->priorities[id] = 0;
		return;
	}
	if(answer == 200 && this->waitingForTechnician) {
		this->priorities[id] = 1;
		return;
	}
	else if(answer == 200 && (numbOfReaders > this->customersCount)){
		this->priorities[id] = 1; // 1 - zezwolenie (wygrana walka)
		return;
	}
	else if(answer == 200 && (numbOfReaders < this->customersCount)){
		this->priorities[id] = 0; //0 - brak zezwolenia (przegrana walka)
		return;
	}
	else if(answer == 200 && numbOfReaders == this->customersCount && id >= this->id){
		this->priorities[id] = 1;
		return;
	}
	else if(answer == 200 && numbOfReaders == this->customersCount && id < this->id) {
		this->priorities[id] = 0;
		return;
	}
}


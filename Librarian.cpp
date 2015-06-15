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
	int customersCount;
	list<int> processesIds;
	int priorities[]; //[0] brak zgody [1] wygrana walka [2] zezwolenie
        list<MPC> freeMPCs;
 public:
	Librarian(int id, int size);
	void setCustomersCount(int count);	
};

Librarian::Librarian(int id, int size){
	 srand(time(0));
         this->id = id;
	 this->customersCount = rand()%id + 7;
	 this->priorities[size];
	 this->priorities[id] = 1; //sam sobie zezwalam
	 cout<<"Librarian o id: "<<id<<" i liczbie klientow: "<<this->customersCount<<endl; 
}

void Librarian::setCustomersCount(int count) {
	this->customersCount = count;	
}


void ReadAnswer(int id, int numbOfReaders, int answer){ 
	printf("Metoda ReadAnswer()\n");
	if(answer == 100){ // 100 - kod dla odpowiedzi "agree"
		printf("heh");
	}
}



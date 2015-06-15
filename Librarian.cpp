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
         this->id = id;
	 this->customersCount = size;
	 this->priorities[size];
	 this->priorities[id] = 1; //sam sobie zezwalam 
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

int main(int argc, char **argv)
{
	int tid, size, len;
	char processor[100];
	MPI_Status status;
	

	MPI::Init(argc, argv);
	size=MPI::COMM_WORLD.Get_size();
	tid=MPI::COMM_WORLD.Get_rank();;
	MPI::Get_processor_name(processor, len);

	srand(time(0));
	int customersCount = rand()%tid + 7;
	cout<<"Wylosowalem: " << customersCount<<endl;

	Librarian librarian(tid, size);
	librarian.setCustomersCount(customersCount);

	printf("Hello! My name is %s (%d of %d)\n", processor, tid, size);
	
	int msg[2]; //[0] id nadawcy [1] liczba zalegających czytelników [2] kod wiadomosci
	if (tid == ROOT){
		for(int i=1; i<size; i++){
			msg[0] = 100;
			msg[1] = tid;
			MPI_Send( msg, 2, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
			printf("ROOT wyslal!\n");
		}
	}else{
		MPI_Recv( msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		printf("Received(%d): %d from %d\n", tid, msg[0], msg[1]);
	}

	MPI::Finalize();
}


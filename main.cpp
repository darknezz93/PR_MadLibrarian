#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <list>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include "Librarian.cpp"
#define ROOT 0
#define MSG_TAG 100


int main(int argc, char **argv)
{
	int tid, size, len;
	char processor[100];
	//MPI_Status status;


	MPI::Init(argc, argv);
	size = MPI::COMM_WORLD.Get_size();
	tid = MPI::COMM_WORLD.Get_rank();
	MPI::Get_processor_name(processor, len);

	Librarian *L1 = new Librarian(tid, size);

	printf("Hello! My name is %s (%d of %d)\n", processor, tid, size);

	//int msg[3]; 
	
	L1->sendRequests();
	L1->waitForAnswears();
	L1->canEnter();
	L1->accessMPC();
	
	
	//[0] id nadawcy [1] liczba zalegaj¹cych czytelników [2] kod wiadomosci
	/*if (tid == ROOT){
		for (int i = 1; i<size; i++){
			msg[0] = 100;
			msg[1] = tid;
			MPI_Send(msg, 2, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
			printf("ROOT wyslal!\n");
		}
	}
	else{
		MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		printf("Received(%d): %d from %d\n", tid, msg[0], msg[1]);
	} */

	MPI::Finalize();
}

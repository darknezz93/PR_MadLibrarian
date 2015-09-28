#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <list>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include "Librarian.cpp"
#include <thread>
#define ROOT 0
#define MSG_TAG 100

void task2() { 
    // do stuff
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

	Librarian *L1 = new Librarian(tid, size);

	printf("Hello! My name is %s (%d of %d), clients: %d, active: %d\n", processor, tid, size, L1->getCustomersCount(),L1->isActive());

	sleep(1);

	
			L1->notifyAboutActivity();
			L1->gatherActiveProcesses();

			//thread updateMPC(&Librarian::waitForMPCUpdate, L1);
			for(int i = 1 ; i < 4; i++) {
				printf("Iteracja: %d ||||||||||||||||||||||||||||||||||||||\n", i);
				sleep(1);		
				L1->sendRequests();
				L1->waitForAnswears();
				L1->canEnter();
				L1->accessMPC();
				L1->updateMPCArray();
				sleep(2);
			}	
	
	 
	//updateMPCArray.join();

	MPI::Finalize(); 
}

#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <list>
#include "MPC.cpp"

using namespace std;

class Librarian {
 
 public:
 	int id;
	int customersCount;
	list<int> processesIds;
        list<MPC> freeMPCs;
	Librarian(int id);	
};

Librarian::Librarian(int id){
         this->id = id;
}


#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <list>
#include <ctime>
#include <cstdlib>
#include <unistd.h>

using namespace std;

class Technician {

  public:
  	int id;
	bool free;
	int capacity;
	
	Technician(int id);
};

Technician::Technician(int id) {
	this->id = id;
	this->free = true;
	this->capacity = rand()%(id+30) + 7;
}


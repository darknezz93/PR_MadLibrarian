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

  private:
  	int id;
	bool free;
	int currentLibrarianId;

  public:		
	Technician(int id, bool free, int currentLibrarianId);
	Technician();
	int getId();
	bool isFree();
	void setFree(bool free);
	int getCurrentLibrarianId();
	void setCurrentLibrarianId(int id);
};

Technician::Technician() {

}

Technician::Technician(int id, bool free, int currentLibrarianId) {
	this->id = id;
	this->free = free;
	this->currentLibrarianId = currentLibrarianId;
}

int Technician::getId() {
	return this->id;
}

bool Technician::isFree() {
	if(this->free) {
		return true;
	}
	return false;
}

void Technician::setFree(bool free) {
	this->free = free;
}

int Technician::getCurrentLibrarianId() {
	return this->currentLibrarianId;
}

void Technician::setCurrentLibrarianId(int id) {
	this->currentLibrarianId = id;
}



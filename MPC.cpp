#include <mpi.h>
#include <iostream>

using namespace std;

class MPC {

  private:
  	int id;
	bool free;
	int servedCustomers;
	int currentLibrarianId;
  public:	
	MPC();
	MPC(int id, bool free, int servedCustomers);
	void set(int id, bool free, int customers);
	bool isFree();
	void setFree(bool free);
	int getId();
	void setId(int id);
	int getServedCustomers();
	void setServedCustomers(int servedCustomers);
	void addServedCustomers(int count);
	void setCurrentLibrarianId(int id);
	int getCurrentLibrarianId();

};

MPC::MPC() {

}

MPC::MPC(int id, bool free, int servedCustomers){
	this->id= id;
	this->free = free;
	this->servedCustomers = servedCustomers;
	this->currentLibrarianId = 500000;
}


void MPC::setId(int id) {
	this->id = id;
}

void MPC::set(int id, bool free, int customers) {
	this->id= id;
	this->free = free;
	this->servedCustomers = customers;
}

bool MPC::isFree() {
	return this->free;
}

void MPC::setFree(bool free) {
	this->free = free;
}

int MPC::getId() {
	return this->id;
}

int MPC::getServedCustomers() {
	return this->servedCustomers;
}

void MPC::setServedCustomers(int servedCustomers) {
	this->servedCustomers = servedCustomers;
}

void MPC::addServedCustomers(int count) {
	this->servedCustomers += count;
}

void MPC::setCurrentLibrarianId(int id) {
	this->currentLibrarianId = id;
}

int MPC::getCurrentLibrarianId() {
	return this->currentLibrarianId;
}



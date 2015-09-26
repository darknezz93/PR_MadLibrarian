#include <mpi.h>
#include <iostream>

using namespace std;

class MPC {

  public:
  	int id;
	bool free;
	int servedCustomers;

	MPC();
	MPC(int id, bool free, int servedCustomers);
	void setId(int id);
	void set(int id, bool free, int customers);
	bool isFree();
};

MPC::MPC() {

}

MPC::MPC(int id, bool free, int servedCustomers){
	this->id= id;
	this->free = free;
	this->servedCustomers = servedCustomers;
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



#include <mpi.h>
#include <iostream>

using namespace std;

class MPC {

  public:
  	int id;
	bool free;
	
	MPC();
	void setId(int id);
};


MPC::MPC() {
	this->free = true;
}

void MPC::setId(int id) {
	this->id = id;
}



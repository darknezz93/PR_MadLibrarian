#include <mpi.h>
#include <iostream>

using namespace std;

class MPC {

  public:
  	int id;
	bool free;
	
	MPC(int id);
};

MPC::MPC(int id) {
	this->id = id;
	this->free = true;
}



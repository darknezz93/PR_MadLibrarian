#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "Librarian.cpp"

using namespace std;

int main(int argc, char** argv) {
  int size;
  int rank;

  MPI_Init(&argc, &argv);

  size = MPI::COMM_WORLD.Get_size();
  rank = MPI::COMM_WORLD.Get_rank();

  Librarian librarian(rank);
  
  return 0;
}


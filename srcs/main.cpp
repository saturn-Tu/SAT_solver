#include "sat_solver.h"
#include <iostream>
using namespace std;

int main(int argc, char** argv) {
  if(argc != 2) {
    cout << "wrong input format!\n";
    cout << "./yasat [input.cnf]\n";
  }
  sat_solver solver;
  solver.init_clauses(argv[1]);
  solver.init_2literal_watch();
  return 0;
}
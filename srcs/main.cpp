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
  cout << "Ori 2-literal-watch\n";
  solver.print2literal_watch();
  solver.printPosNegWatch();
  bool sat_flg = solver.DPLL_start();
  if(sat_flg) {
    cout << "SAT\n";
    solver.printAssignedValue();
  }
  else cout << "UNSAT\n";
  return 0;
}
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
  //cout << "Ori 2-literal-watch\n";
  //solver.print2literal_watch();
  //solver.printPosNegWatch();
  solver.calculateJW_Score();
  solver.sat_flg = solver.DPLL_start();
  if(solver.sat_flg) {
    cout << "SAT\n";
    //solver.printAssignedValue();
  }
  else cout << "UNSAT\n";
  string f_name(argv[1]);
  f_name = f_name.substr(0, f_name.size()-4)+".sat";
  solver.outputSAT_File(f_name.c_str());
  return 0;
}
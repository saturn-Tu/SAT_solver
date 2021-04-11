#include <iostream>
#include "sat_solver.h"
using namespace std;

void sat_solver::init_clauses(const char *DIMACS_cnf_file) {
  parse_DIMACS_CNF(clauses, maxVarIndex, DIMACS_cnf_file);
  // construct vars_columns
  vars_columns.resize(maxVarIndex+1);
  for(int c=0; c<clauses.size(); c++) {
    for(int v=0; v<clauses[c].size(); v++) {
      int v_idx = abs(clauses[c][v]);
      vars_columns[v_idx].insert(c);
    }
  }
}
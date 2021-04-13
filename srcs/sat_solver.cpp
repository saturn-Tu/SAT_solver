#include <iostream>
#include "sat_solver.h"
using namespace std;

void sat_solver::init_clauses(const char *DIMACS_cnf_file) {
  vector< vector<int> > tmp_clauses;
  parse_DIMACS_CNF(tmp_clauses, maxVarIndex, DIMACS_cnf_file);
  // construct vars_columns, sparse matrix
  vars_columns.resize(maxVarIndex+1);
  clauses.resize(tmp_clauses.size());
  for(int c=0; c<tmp_clauses.size(); c++) {
    clauses[c].resize(maxVarIndex+1);
    for(int v=0; v<tmp_clauses[c].size(); v++) {
      int v_idx = abs(tmp_clauses[c][v]);
      vars_columns[v_idx].insert(c);
      clauses[c][v_idx] = (tmp_clauses[c][v]>0) ? 1 : -1;
    }
  }
}
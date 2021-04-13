#include <iostream>
#include "sat_solver.h"
using namespace std;

void sat_solver::init_clauses(const char *DIMACS_cnf_file) {
  vector< vector<int> > tmp_clauses;
  parse_DIMACS_CNF(tmp_clauses, maxVarIndex, DIMACS_cnf_file);
  // construct vars_columns, sparse matrix
  assigned_value.resize(maxVarIndex+1);
  vars_columns.resize(maxVarIndex+1);
  clauses.resize(tmp_clauses.size());
  for(int c=0; c<tmp_clauses.size(); c++) {
    for(int v=0; v<tmp_clauses[c].size(); v++) {
      int v_idx = abs(tmp_clauses[c][v]);
      vars_columns[v_idx].insert(c);
      clauses[c][v_idx] = (tmp_clauses[c][v]>0) ? 1 : 0;
    }
  }
}

void sat_solver::init_2literal_watch() {
  watch_vars.resize(clauses.size());
  for(int c=0; c<clauses.size(); c++) {
    auto& clause = clauses[c];
    watch_vars[c].first = clauses[c].end();
    watch_vars[c].second = clauses[c].end();
    uint8_t counter = 0;
    for(auto itr = clause.begin(); itr != clause.end() ; itr++) {
      if(counter == 0)
        watch_vars[c].first = itr;
      else if(counter == 1)
        watch_vars[c].second = itr;
      else break;
      counter++;
    }
  }
}
#include "parser.h"
#include <set>
#include <cstdlib>

class sat_solver {
public:
  int maxVarIndex;
  // sparse matrix
  vector< vector<int> > clauses;
  // clause indexs where specific variable exist, <var_idx, clauses_idx>
  vector< std::set<int> > vars_columns;

  //-------------- function -----------------
  void init_clauses(const char *DIMACS_cnf_file);
  void DPLL();
};
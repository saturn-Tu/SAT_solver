#include "parser.h"
#include <set>
#include <cstdlib>

class sat_solver {
public:
  vector< vector<int> > clauses;
  int maxVarIndex;
  // clause indexs where specific variable exist, <var_idx, clauses_idx>
  vector< std::set<int> > vars_columns;

  //-------------- function -----------------
  void init_clauses(const char *DIMACS_cnf_file);
  void DPLL();
};
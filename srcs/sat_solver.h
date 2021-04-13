#include "parser.h"
#include <set>
#include <map>
#include <utility>
#include <cstdlib>
#include <cstdint>

class sat_solver {
public:
  int maxVarIndex;
  vector<int> assigned_value;
  // sparse matrix, 0 means negation, 1 means positive
  vector< std::map<int,bool> > clauses;
  // clause indexs where specific variable exist, <var_idx, clauses_idx>
  vector< std::set<int> > vars_columns;
  // 2-literal watch member
  // list of clauses in which x, x' is a watched variable
  vector< std::set<int> > pos_watched, neg_watched;
  vector< std::pair<std::map<int,bool>::iterator, std::map<int,bool>::iterator> > watch_vars;

  //-------------- function -----------------
  void init_clauses(const char *DIMACS_cnf_file);
  void DPLL();
  void init_2literal_watch();
};
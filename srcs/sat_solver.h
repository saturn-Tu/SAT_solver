#include "parser.h"
#include <set>
#include <unordered_map>
#include <utility>
#include <cstdlib>
#include <cstdint>
#include <queue>
#define NOT_ASSIGNED 2
#define SAT 1
#define UNSAT 0

class sat_solver {
public:
  int maxVarIndex;
  vector<uint8_t> assigned_value;
  // sparse matrix, 0 means negation, 1 means positive
  vector< std::unordered_map<int,bool> > clauses;
  // clause indexs where specific variable exist, <var_idx, clauses_idx>
  vector< std::set<int> > vars_columns;
  // 2-literal watch member
  // list of clauses in which x, x' is a watched variable, row is variable index, set is clause idx
  vector< std::set<int> > pos_watched, neg_watched;
  vector< std::pair<std::unordered_map<int,bool>::iterator, std::unordered_map<int,bool>::iterator> > watch_vars;

  //-------------- function -----------------
  void init_clauses(const char *DIMACS_cnf_file);
  bool DPLL(int var, bool value);
  void init_2literal_watch();
  // return status from 1~4
  // case1, have found another watched variable
  // case2, only remain one watched variable -> unit clause
  // case3, another watched variable is true, clause is resolved
  // case4, conflict clause
  int update_2literal_watch(int clause_idx, int var, bool value, std::queue< std::pair<int, bool> >& pending_literals, vector<int>& erase_watchs);
  void print2literal_watch();
  void printPosNegWatch();
  void printAssignedValue();
};
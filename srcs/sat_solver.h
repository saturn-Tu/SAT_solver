#include "parser.h"
#include <set>
#include <unordered_map>
#include <utility>
#include <cstdlib>
#include <cstdint>
#include <queue>
#include <list>
#include <cmath>
#include <algorithm>
#include <string>
#include <fstream>
#include <cassert>
#define NOT_ASSIGNED 2
#define SAT 1
#define UNSAT 0

class J_W_Socre {
public:
  int var;
  float pos_value, neg_value, max_value;
  J_W_Socre() {
    pos_value = 0; neg_value = 0, max_value = 0;
  }
  bool operator >(const J_W_Socre &other) const {
    if(max_value == other.max_value)
      return std::min(pos_value, neg_value) > std::min(other.pos_value, other.neg_value);
    return max_value > other.max_value;
  }
};

class ConflictPoint {
public:
  int var_idx;
  int clause_idx;
  ConflictPoint() {}
  ConflictPoint(int var, int clause) : var_idx(var), clause_idx(clause) {}
};

class sat_solver {
public:
  int maxVarIndex;
  vector<uint8_t> assigned_value;
  vector<int> vars_level;
  // sparse matrix, 0 means negation, 1 means positive
  vector< std::unordered_map<int,bool> > clauses;
  // clause indexs where specific variable exist, <var_idx, clauses_idx>
  vector< std::set<int> > vars_columns;
  // 2-literal watch member
  // list of clauses in which x, x' is a watched variable, row is variable index, set is clause idx
  vector< std::set<int> > pos_watched, neg_watched;
  vector< std::pair<std::unordered_map<int,bool>::iterator, std::unordered_map<int,bool>::iterator> > watch_vars;
  // variable order
  vector<J_W_Socre> var_score;
  bool sat_flg;
  // for Non-chronological backtracking, return to desinate level
  int return_level;
  // for random restart
  int backtrack_counter;
  int backtrack_bound = 500;
  int total_backtrack_counter;
  //-------------- function -----------------
  void init_clauses(const char *DIMACS_cnf_file);
  bool DPLL(int var, bool value, int current_level);
  bool DPLL_start();
  void init_2literal_watch();
  void init_2literal_watch_clause(int clause_idx);
  // return status from 1~4
  // case1, have found another watched variable
  // case2, only remain one watched variable -> unit clause
  // case3, another watched variable is true, clause is resolved
  // case4, conflict clause
  int update_2literal_watch(int clause_idx, int var, bool value, std::queue< std::pair<int, bool> >& pending_literals, 
    vector<int>& erase_watchs, std::list<ConflictPoint>& conflict_points);
  void print2literal_watch();
  void printPosNegWatch();
  void printAssignedValue();
  // calculate Jeroslaw-Wang Score
  void calculateJW_Score();
  void outputSAT_File(const char *sat_file);
  void firstUIP(std::unordered_map<int,bool>& conflict_clause, std::list<ConflictPoint>& conflict_points, int current_level);
  bool checkResolveClause(std::unordered_map<int,bool>& clause, std::unordered_map<int,bool>& conflict_clause);
};
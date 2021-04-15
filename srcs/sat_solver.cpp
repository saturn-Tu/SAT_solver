#include <iostream>
#include "sat_solver.h"
using namespace std;

void sat_solver::init_clauses(const char *DIMACS_cnf_file) {
  vector< vector<int> > tmp_clauses;
  parse_DIMACS_CNF(tmp_clauses, maxVarIndex, DIMACS_cnf_file);
  // construct vars_columns, sparse matrix
  assigned_value.resize(maxVarIndex+1, NOT_ASSIGNED);
  vars_columns.resize(maxVarIndex+1);
  pos_watched.resize(maxVarIndex+1);
  neg_watched.resize(maxVarIndex+1);
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
      // initial pos/neg_watched list
      if(itr->second == 1)
        pos_watched[itr->first].insert(c);
      else 
        neg_watched[itr->first].insert(c);
      counter++;
    }
  }
}

bool sat_solver::DPLL(int var, bool value) {
  // reserver for back_track
  vector<uint8_t> ori_assigned_value = assigned_value;
  // update 2-literal watch variable
  if(value) {
    for(auto clause_idx : neg_watched[var]) {
      int status = update_2literal_watch(clause_idx, var, value);
      if(status == 4) {
        // case4: conflict!, return UNSAT
        return 0;
      }
    }
  } else {
    for(auto clause_idx : pos_watched[var]) {
      int status = update_2literal_watch(clause_idx, var, value);
      if(status == 4) {
        // case4: conflict!, return UNSAT
        return 0;
      }
    }
  }
  // choose an unassigned variable 

}

int sat_solver::update_2literal_watch(int clause_idx, int var, bool value) {
  auto& clause = clauses[clause_idx];
  auto& watch_var = watch_vars[clause_idx];
  unordered_map<int,bool>::iterator now_watch_var, watch_var2;
  bool use_var1_flg = 0;
  if(watch_var.first->first == var) {
    now_watch_var = watch_var.first;
    watch_var2 = watch_var.second;
    use_var1_flg = 1;
  } else if(watch_var.second->first == var) {
    now_watch_var = watch_var.second;
    watch_var2 = watch_var.first;
    use_var1_flg = 0;
  } else return;
  // find another watched variable
  auto itr = now_watch_var;
  itr++;
  for(; itr != now_watch_var; itr++) {
    if(itr == clause.end())
      itr == clause.begin();
    int var_idx = itr->first;
    if(assigned_value[var_idx] == NOT_ASSIGNED && itr != watch_var2)
      break;
  }
  if(itr != now_watch_var) {
    // case1, have found another watched variable
    // update pos/neg_watched list
    if(value)
      neg_watched[var].erase(clause_idx);
    else
      pos_watched[var].erase(clause_idx);
    if(itr->second)
      pos_watched[itr->first].insert(clause_idx);
    else
      neg_watched[itr->first].insert(clause_idx);
    // update local 2-literal watched variable
    if(use_var1_flg)
      watch_var.first = itr;
    else
      watch_var.second = itr;
    return 1;
  } else {
    int var2_idx = watch_var2->first;
    if(assigned_value[var2_idx] == NOT_ASSIGNED) {
      // case2, only remain one watched variable -> unit clause
      
      return 2;
    } else if (assigned_value[var2_idx] == watch_var2->second ) {
      // case3, another watched variable is true, clause is resolved
      return 3;
    } else {
      // case4, conflict clause
      return 4;
    }
    
  }
}
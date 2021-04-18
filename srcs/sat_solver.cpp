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
      if(counter == 0) {
        watch_vars[c].first = itr;
        watch_vars[c].second = itr;
      }
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
  queue< pair<int, bool> > pending_literals;
  pending_literals.emplace(var, value);
  while(!pending_literals.empty()) {
    pair<int, bool> literal = pending_literals.front();
    pending_literals.pop();
    assigned_value[literal.first] = literal.second;
    // update 2-literal watch variable
    vector<int> erase_watchs;
    int status;
    if(literal.second) {
      for(auto clause_idx : neg_watched[literal.first]) {
        //cout << "NEG\n";
        status = update_2literal_watch(clause_idx, literal.first, literal.second, pending_literals, erase_watchs);
        //print2literal_watch();
        //cout << "status = " << status << endl;
        // case4: conflict!, return UNSAT
        if(status == 4) break;
      }
      for(auto& clause_idx : erase_watchs) {
        neg_watched[literal.first].erase(clause_idx);
      }
      if(status == 4) return UNSAT;
    } else {
      for(auto clause_idx : pos_watched[literal.first]) {
        //cout << "POS\n";
        status = update_2literal_watch(clause_idx, literal.first, literal.second, pending_literals, erase_watchs);
        //print2literal_watch();
        //cout << "status = " << status << endl;
        // case4: conflict!, return UNSAT
        if(status == 4) break;
      }
      for(auto& clause_idx : erase_watchs) {
        pos_watched[literal.first].erase(clause_idx);
      }
      if(status == 4) return UNSAT;
    }
    //printPosNegWatch();
    //int a; cin >> a;
    //cout << "finish" << endl;
  }
  // check clauses is SAT/UNSAT
  bool sat_flag = 1;
  for(auto& watch_var : watch_vars) {
    // clause is SAT
    if(watch_var.first->second == assigned_value[watch_var.first->first] || 
      watch_var.second->second == assigned_value[watch_var.second->first])
      continue;
    sat_flag = 0;
    // clause has NOT_ASSIGNED variable, still has chance to become SAT
    if(assigned_value[watch_var.first->first] == NOT_ASSIGNED || 
      assigned_value[watch_var.second->first] == NOT_ASSIGNED )
      continue;
    return UNSAT;
  }
  if(sat_flag == 1)
    return SAT;
  // reserver for back_track
  vector<uint8_t> ori_assigned_value = assigned_value;
  // choose an unassigned variable 
  for(int n=1; n<var_score.size(); n++) {
    int& var_idx = var_score[n].var;
    if(assigned_value[var_idx] == NOT_ASSIGNED) {
      bool value = (var_score[n].pos_value > var_score[n].neg_value);
      //cout << "first try\n";
      bool sat_flg = DPLL(var_idx, value);
      if(sat_flg == SAT)
        return SAT;
      assigned_value = ori_assigned_value;
      //cout << "second try\n";
      sat_flg = DPLL(var_idx, !value);
      return sat_flg;
    }
  }
  return UNSAT;
}

bool sat_solver::DPLL_start() {
  vector<uint8_t> ori_assigned_value = assigned_value;
  // choose an unassigned variable 
  for(int n=1; n<var_score.size(); n++) {
    int& var_idx = var_score[n].var;
    if(assigned_value[var_idx] == NOT_ASSIGNED) {
      bool value = (var_score[n].pos_value > var_score[n].neg_value);
      //cout << "first try\n";
      bool sat_flg = DPLL(var_idx, value);
      if(sat_flg == SAT)
        return SAT;
      assigned_value = ori_assigned_value;
      //cout << "second try\n";
      sat_flg = DPLL(var_idx, !value);
      return sat_flg;
    }
  }
  return UNSAT;
}

int sat_solver::update_2literal_watch(int clause_idx, int var, bool value, queue< pair<int, bool> >& pending_literals, vector<int>& erase_watchs) {
  //cout << "choose var " << var << ", val = " << value << ", clause idx = " << clause_idx << endl;
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
  }
  // find another watched variable
  auto itr = now_watch_var;
  itr++;
  if(itr == clause.end())
    itr = clause.begin();
  for(; itr != now_watch_var;) {
    int var_idx = itr->first;
    if((assigned_value[var_idx]==NOT_ASSIGNED || assigned_value[var_idx]==clause[var_idx]) && itr != watch_var2)
      break;
    itr++;
    if(itr == clause.end())
      itr = clause.begin();
  }
  if(itr != now_watch_var) {
    // case1, have found another watched variable
    // update pos/neg_watched list
    erase_watchs.emplace_back(clause_idx);
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
      pending_literals.emplace(var2_idx, clause[var2_idx]);
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

void sat_solver::print2literal_watch() {
  for(int n=0; n<watch_vars.size(); n++) {
    auto& watch_var = watch_vars[n];
    cout << "c" << n << ":\t";
    cout << watch_var.first->first << " " << watch_var.first->second << " | ";
    cout << watch_var.second->first << " " << watch_var.second->second << "\n";
  }
}

void sat_solver::printPosNegWatch() {
  printAssignedValue();
  cout << "Pos watch list\n";
  for(int n=1; n<pos_watched.size(); n++) {
    cout << "var" << n << " ";
    for(auto pos : pos_watched[n])
      cout << pos << " ";
    cout << endl;
  }
  cout << "Neg watch list\n";
  for(int n=1; n<neg_watched.size(); n++) {
    cout << "var" << n << " ";
    for(auto neg : neg_watched[n])
      cout << neg << " ";
    cout << endl;
  }
}

void sat_solver::printAssignedValue() {
  cout << "Assigned value\n";
  for(int n=1; n<assigned_value.size(); n++)
    cout << n << " ";
  cout << "\n";
  for(int n=1; n<assigned_value.size(); n++)
    cout << (int)assigned_value[n] << " ";
  cout << "\n";
}

void sat_solver::calculateJW_Score() {
  var_score.resize(maxVarIndex+1);
  for(int n=1; n<var_score.size(); n++)
    var_score[n].var = n;
  // calculate JW Score
  for(auto& clause : clauses) {
    for(auto literal : clause) {
      auto& var = var_score[literal.first];
      float score = exp2(-(int)clause.size());
      if(literal.second) var.pos_value += score;
      else var.neg_value += score;
    }
  }
  // update max value at each literal score
  for(int n=1; n<var_score.size(); n++)
    var_score[n].max_value = max(var_score[n].pos_value, var_score[n].neg_value);
  sort(var_score.begin()+1, var_score.end(), greater<J_W_Socre>());
  //for(int n=1; n<var_score.size(); n++) {
  //  cout << var_score[n].var << ": " << var_score[n].pos_value << " " << var_score[n].neg_value << endl;
  //}
}

void sat_solver::outputSAT_File(const char* sat_file) {
  fstream output_file;
  output_file.open(sat_file, ios::out);
  output_file << "s ";
  if(sat_flg) {
    output_file << "SATISFIABLE\n";
    output_file << "v ";
    for(int n=1; n<assigned_value.size(); n++) {
      // assign "NOT_ASSIGNED" as "0"
      if(assigned_value[n] == NOT_ASSIGNED) 
        output_file << "0 ";
      else
        output_file << (int)assigned_value[n] << " ";
    }
  } else {
    output_file << "UNSATISFIABLE\n";
  }
  output_file.close();
}
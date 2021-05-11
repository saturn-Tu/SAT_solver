#include <iostream>
#include "sat_solver.h"
using namespace std;

void sat_solver::init_clauses(const char *DIMACS_cnf_file) {
  vector< vector<int> > tmp_clauses;
  parse_DIMACS_CNF(tmp_clauses, maxVarIndex, DIMACS_cnf_file);
  // construct vars_columns, sparse matrix
  assigned_value.resize(maxVarIndex+1, NOT_ASSIGNED);
  vars_level.resize(maxVarIndex+1, 0);
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
    init_2literal_watch_clause(c);
  }
}

void sat_solver::init_2literal_watch_clause(int clause_idx) {
  auto& clause = clauses[clause_idx];
  watch_vars[clause_idx].first = clauses[clause_idx].end();
  watch_vars[clause_idx].second = clauses[clause_idx].end();
  uint8_t counter = 0;
  for(auto itr = clause.begin(); itr != clause.end() ; itr++) {
    if(counter == 0) {
      watch_vars[clause_idx].first = itr;
      watch_vars[clause_idx].second = itr;
    }
    else if(counter == 1)
      watch_vars[clause_idx].second = itr;
    else break;
    // initial pos/neg_watched list
    if(itr->second == 1)
      pos_watched[itr->first].insert(clause_idx);
    else 
      neg_watched[itr->first].insert(clause_idx);
    counter++;
  }
}

bool sat_solver::DPLL(int var, bool value, int level) {
  queue< pair<int, bool> > pending_literals;
  pending_literals.emplace(var, value);
  list<ConflictPoint> conflict_points;
  cout << "\nlevel: " << level << " " << var << " " << value << endl;
  while(!pending_literals.empty()) {
    pair<int, bool> literal = pending_literals.front();
    pending_literals.pop();
    cout << literal.first << " " << endl;
    assigned_value[literal.first] = literal.second;
    vars_level[literal.first] = level;
    // update 2-literal watch variable
    vector<int> erase_watchs;
    int status, conflict_clause_idx = 0;
    if(literal.second) {
      for(auto clause_idx : neg_watched[literal.first]) {
        cout << "NEG" << endl;
        status = update_2literal_watch(clause_idx, literal.first, literal.second, pending_literals, erase_watchs, conflict_points);
        //print2literal_watch();
        cout << "status = " << status << endl;
        // case4: conflict!, return UNSAT
        if(status == 4) {
          conflict_clause_idx = clause_idx;
          break;
        }
      }
      for(auto& clause_idx : erase_watchs) {
        neg_watched[literal.first].erase(clause_idx);
      }
    } else {
      for(auto clause_idx : pos_watched[literal.first]) {
        cout << "POS" << endl;
        cout << "c_idx: " << clause_idx << " ";
        cout << "clause size: " << clauses.size() << endl;
        status = update_2literal_watch(clause_idx, literal.first, literal.second, pending_literals, erase_watchs, conflict_points);
        //print2literal_watch();
        cout << "status = " << status << endl;
        // case4: conflict!, return UNSAT
        if(status == 4) {
          conflict_clause_idx = clause_idx;
          break;
        }
      }
      for(auto& clause_idx : erase_watchs) {
        pos_watched[literal.first].erase(clause_idx);
      }
    }
    if(status == 4) {
      // doing firstUIP to learn new constraint, then return
      auto conflict_clause = clauses[conflict_clause_idx];
      firstUIP(conflict_clause, conflict_points, level);
      return UNSAT;
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
  vector<int> ori_vars_level = vars_level;
  // choose an unassigned variable 
  for(int n=1; n<var_score.size(); n++) {
    int& var_idx = var_score[n].var;
    if(assigned_value[var_idx] == NOT_ASSIGNED) {
      bool value = (var_score[n].pos_value > var_score[n].neg_value);
      //cout << "first try\n";
      bool sat_flg = DPLL(var_idx, value, level+1);
      if(sat_flg == SAT)
        return SAT;
      assigned_value = ori_assigned_value;
      vars_level = ori_vars_level;
      //cout << "second try\n";
      sat_flg = DPLL(var_idx, !value, level+1);
      return sat_flg;
    }
  }
  return UNSAT;
}

bool sat_solver::DPLL_start() {
  vector<uint8_t> ori_assigned_value = assigned_value;
  vector<int> ori_vars_level = vars_level;
  // choose an unassigned variable 
  for(int n=1; n<var_score.size(); n++) {
    int& var_idx = var_score[n].var;
    if(assigned_value[var_idx] == NOT_ASSIGNED) {
      bool value = (var_score[n].pos_value > var_score[n].neg_value);
      //cout << "first try\n";
      bool sat_flg = DPLL(var_idx, value, 0);
      if(sat_flg == SAT)
        return SAT;
      assigned_value = ori_assigned_value;
      vars_level = ori_vars_level;
      //cout << "second try\n";
      sat_flg = DPLL(var_idx, !value, 0);
      return sat_flg;
    }
  }
  return UNSAT;
}

int sat_solver::update_2literal_watch(int clause_idx, int var, bool value, queue< pair<int, bool> >& pending_literals, 
  vector<int>& erase_watchs, list<ConflictPoint>& conflict_points) {
  cout << "choose var " << var << ", val = " << value << ", clause idx = " << clause_idx << endl;
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
      conflict_points.emplace_front(var2_idx, clause_idx);
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
  cout << "size " << pos_watched.size() << endl;
  for(int n=1; n<pos_watched.size(); n++) {
    cout << "var" << n << " ";
    cout << "s:" << pos_watched[n].size() << endl;
    for(auto pos : pos_watched[n])
      cout << pos << " ";
    cout << endl;
  }
  cout << "Neg watch list" << endl;
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

bool sat_solver::checkResolveClause(std::unordered_map<int,bool>& clause, std::unordered_map<int,bool>& conflict_clause) {
  for(auto var1 : clause) {
    for(auto var2 : conflict_clause) {
      if(var1.first == var2.first && var1.second == !var2.second)
        return 1;
    }
  }
  return 0;
}

void sat_solver::firstUIP(std::unordered_map<int,bool>& conflict_clause, list<ConflictPoint>& conflict_points, 
  int current_level) {
  bool change_flg = 0;
  cout << "cur level: " << current_level << endl;
  cout << "First conf: ";
  for(auto var : conflict_clause) {
    if(var.second)
      cout << var.first << " ";
    else 
      cout << "-" << var.first << " ";
  }
  cout << "\n";

  while(1) {
    // check conflict_clause has more than one literal assigned at current decision level
    int counter = 0;
    int current_var = 0;
    cout << "conflict_clause var level\n";
    for(auto var : conflict_clause) {
      cout << var.first << ", " << vars_level[var.first] << endl;
      if(vars_level[var.first] == current_level) {
        current_var = var.first;
        counter++;
      }
    }
    cout << "before counter " << counter << endl;
    if(counter < 2)
      break;
    // find last relation clause
    if(conflict_points.empty())
      return;
    auto it=conflict_points.begin();
    cout << "Check errer" << endl;
    for (; it!=conflict_points.end(); ++it) {
      auto& clause = clauses[it->clause_idx];
      cout << "before checkResolveClause" << endl;
      if( checkResolveClause(clause, conflict_clause) )
        break;
    }
    cout << "after checkResolveClause" << endl;
    assert(it != conflict_points.end());
    auto conflictPoint = *it;
    conflict_points.erase(it);
    auto past_clause = clauses[conflictPoint.clause_idx];
    // handle past_clause and conflict_clause are same
    if(past_clause == conflict_clause)
      continue;
    int clause_var = conflictPoint.var_idx;
    cout << "p: " << clause_var << endl;
    cout << "conf: ";
    for(auto var : conflict_clause) {
      if(var.second)
        cout << var.first << " ";
      else 
        cout << "-" << var.first << " ";
    }
    cout << "\npast: ";
    for(auto var : past_clause) {
      if(var.second)
        cout << var.first << " ";
      else 
        cout << "-" << var.first << " ";
    }
    cout << "\n";
    // check var is in both clause
    assert(past_clause.find(clause_var) != past_clause.end());
    assert(conflict_clause.find(clause_var) != conflict_clause.end());
    // check value in two clause is different
    assert(past_clause[clause_var] == !conflict_clause[clause_var]);
    // doing resolve
    for(auto itr : past_clause) {
      conflict_clause.insert(itr);
    }
    conflict_clause.erase(clause_var);
    // print resolve
    cout << "resolve: ";
    for(auto var : conflict_clause) {
      if(var.first == clause_var) continue;
      if(var.second)
        cout << var.first << " ";
      else 
        cout << "-" << var.first << " ";
    }
    cout << "\n";
    change_flg = 1;
  }
  // add firstUIP to new constraint
  cout << "change flg " << change_flg << " " << conflict_clause.size() << endl;
  if(change_flg && conflict_clause.size() < 10) {
    clauses.push_back(conflict_clause);
    if(clauses.size()-1 > 200) {
      int a; cin>> a;
    }
    watch_vars.resize(watch_vars.size()+1);
    init_2literal_watch_clause(clauses.size()-1);
    cout << "*** Add new constraint!" << endl;
    cout << "size: " << clauses.size()-1 << endl;
    printPosNegWatch();
  }
}
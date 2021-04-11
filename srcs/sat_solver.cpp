#include <iostream>
#include "sat_solver.h"
using namespace std;

void sat_solver::parse_file(const char *DIMACS_cnf_file) {
  parse_DIMACS_CNF(clauses, maxVarIndex, DIMACS_cnf_file);
}
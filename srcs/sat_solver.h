#include "parser.h"

class sat_solver {
public:
  vector< vector<int> > clauses;
  int maxVarIndex;

  void parse_file(const char *DIMACS_cnf_file);
};
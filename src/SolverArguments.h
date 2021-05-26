#ifndef _SolverArguments_h
#define _SolverArguments_h

#include <boost/program_options.hpp>

namespace po=boost::program_options;

class SolverArguments
{
  int argc;
  const char **argv;
  po::variables_map vm;
  bool help;
public:
  SolverArguments(int ac, const char **av);
  po::variables_map GetMap() const { return &vm; }
  bool IsHelp() const { return help; }
  bool IsTest() const { return vm.count("test"); }
  bool IsMaterials() const { return vm.count("materials"); }
};

#endif

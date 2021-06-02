#ifndef _OptArguments_h
#define _OptArguments_h

#include <boost/program_options.hpp>

namespace po=boost::program_options;

class OptArguments
{
  int argc;
  const char **argv;
  po::variables_map vm;
  bool help;
public:
  OptArguments(int ac, const char **av);
  po::variables_map GetMap() const { return &vm; }
  bool IsHelp() const { return help; }
  bool IsMaterials() const { return vm.count("mat"); }
  size_t getNLayers() const { return vm["nlayers"].as<size_t>(); }
  size_t getNGen() const { return vm["ngen"].as<size_t>(); }
  size_t getNPrint() const { return vm["nprint"].as<size_t>(); }
  std::vector<std::string> getTail() const { return vm["tail"].as<std::vector<std::string>>(); }
};

#endif

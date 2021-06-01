#ifndef SDEF_h
#define SDEF_h

#include <iostream>

#include <TROOT.h>
#include <TH2D.h>

class SDEF
{
 private:
  std::string fname; //< ROOT file name with source definition
  std::map<char, std::shared_ptr<TH2D>> sdef; //< particle id and sdef map
 public:
  SDEF(const std::string&);
  std::map<char, std::shared_ptr<TH2D>>& getSDEF() { return sdef; }
};

#endif

#ifndef Test_h
#define Test_h

#include <TROOT.h>
#include <TH2D.h>

class Test
{
private:
  const size_t n;
  const size_t nLayers;
  std::map<char, std::shared_ptr<TH2D>> sdef;
public:
  Test(size_t, size_t);
  bool run();
  inline void setSDEF(std::map<char, std::shared_ptr<TH2D>>& s) { sdef = s; }
  inline std::map<char, std::shared_ptr<TH2D>>& getSDEF() { return sdef; }
};

#endif

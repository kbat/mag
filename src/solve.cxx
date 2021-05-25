#include <iostream>
#include <chrono>

#include <boost/program_options.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <sys/ioctl.h>

#include <TROOT.h>
#include <TH2D.h>
#include <TFile.h>

#include "Material.h"
#include "Source.h"
#include "Solver.h"

namespace po=boost::program_options;

class Arguments
{
  int argc;
  const char **argv;
  po::variables_map vm;
  bool help;
public:
  Arguments(int ac, const char **av);
  po::variables_map GetMap() const { return &vm; }
  bool IsHelp() const { return help; }
  bool IsTest() const { return vm.count("test"); }
};

Arguments::Arguments(int ac, const char **av) :
  argc(ac), argv(av), help(false)
{
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  try {
 po::options_description hidden("Positional arguments");
    hidden.add_options()
      ("layers", po::value<std::string>()->default_value(""), "Description of layers");


    //  options(argc, argv);
  po::options_description generic("Generic options", w.ws_col);
    generic.add_options()
      ("help,h", "Show this help message and exit.")
      ("test",  po::value<std::vector<size_t> >()->multitoken(), "Test number to run")
      ;

    std::array<std::string, 4> positional_args{"layers"};
    po::positional_options_description p;
    for (const std::string& pa : positional_args)
      p.add(pa.data(), 1);

    po::options_description all_options("Usage: gam-solve [options] [layers]");
    all_options.add(generic).add(hidden);

    //    po::store(po::parse_command_line(argc, argv, desc), vm);
    auto parsed = po::command_line_parser(argc, argv).options(all_options).positional(p)
      .style(po::command_line_style::allow_short |
             po::command_line_style::short_allow_adjacent |
             po::command_line_style::short_allow_next |
             po::command_line_style::allow_long |
             po::command_line_style::long_allow_adjacent |
             po::command_line_style::long_allow_next |
             po::command_line_style::allow_sticky |
             po::command_line_style::allow_dash_for_short |
             po::command_line_style::allow_long_disguise)
      .run();
  // for (const std::string& pa : positional_args)
  //     {
  //       auto it = std::find_if(parsed.options.begin(), parsed.options.end(),
  //                              [&pa](po::option const& o) {
  //                                return o.string_key == pa;
  //                              });
  //       if ((it == parsed.options.end()) && (pa!="gfile") && (pa!="ghist") ) // gfile and ghist are optional
  //         {
  //           std::cerr << "Error: Missing positional argument \"" <<
  //             pa << "\"\n" << std::endl;
  //           help=true;
  //           break;
  //         }
  //     }

    po::store(parsed, vm);
    try {
      po::notify(vm);
    } catch (boost::program_options::error& e) {
      std::cout << "Error: " << e.what() << "\n";
      exit(1);
    }

    if (help || vm.count("help"))
      {
        help = true;
        std::stringstream stream;
        stream << all_options;
        std::string helpMsg = stream.str();
        boost::algorithm::replace_all(helpMsg, "--", "-");
	boost::algorithm::replace_all(helpMsg, "-layers", " layers");
        std::cout << helpMsg << std::endl;
        return;
      }
  }
  catch(std::exception& e) {
    std::cerr << "hplot ERROR: " << e.what() << "\n";
    exit(1);
  }
  catch(...) {
    std::cerr << "Exception of unknown type!\n";
    exit(2);
  }

  return;
}


bool test(size_t n, size_t nLayers)
/*!
  Run some tests
 */
{
  bool val(false);

  //  std::shared_ptr<Material> m;
  std::vector<std::shared_ptr<Material>> mat;

  auto mTest1 = std::make_shared<Material>("Test", "test/solver/test1.root", 1, 1);
  auto mTest2 = std::make_shared<Material>("Test", "test/solver/test2.root", 2, 1);
  auto mTest3 = std::make_shared<Material>("Test", "test/solver/test3.root", 3, 1);
  auto mTest5 = std::make_shared<Material>("Test", "test/solver/test5.root", 4, 1);
  auto mTest6 = std::make_shared<Material>("Test", "test/solver/test6.root", 5, 1);

  size_t ro=0; // reflection order
  if (n==1) {
    for (size_t i=0; i<nLayers; ++i)
      mat.push_back(mTest1);
  } else if (n==2) {
    for (size_t i=0; i<nLayers; ++i)
      mat.push_back(mTest2);
  } else if (n==3) {
    for (size_t i=0; i<nLayers; ++i)
      mat.push_back(mTest3);
  } else if (n==4) {
    ro = 1;
    for (size_t i=0; i<nLayers; ++i)
      mat.push_back(mTest1);
  } else if (n==5) {
    ro = 1;
    for (size_t i=0; i<nLayers; ++i)
      mat.push_back(mTest5);
  } else if (n==6) {
    ro = 1;
    for (size_t i=0; i<nLayers; ++i)
      mat.push_back(mTest6);
  } else {
    std::cerr << "gam-solver: test error" << std::endl;
    return false;
  }
  auto solver = std::make_shared<Solver>('n', mat[0]->getSDEF(), mat);
  solver->run(ro);
  solver->save("res.root");

  return val;
}

int main(int argc, const char **argv)
{
  std::unique_ptr<Arguments> args = std::make_unique<Arguments>(argc, argv);

  if (args->IsHelp())
    return 0;

  if (args->IsTest()) {
    auto t = args->GetMap()["test"].as<std::vector<size_t> >();
    return test(t[0], t[1]);
  }


  auto poly = std::make_shared<Material>("Polyethylene", "Poly.root", 48, 0.91);
  auto concrete = std::make_shared<Material>("Concrete", "Concrete.root", 49, 2.33578);
  auto b4c = std::make_shared<Material>("B4C", "B4C.root", 47, 2.50608);
  auto steel = std::make_shared<Material>("Stainless304", "Stainless304.root", 3, 7.96703);
  auto W = std::make_shared<Material>("Tungsten", "Tungsten.root", 38, 19.413);

  size_t nLayers = argc == 2 ? atoi(argv[1]) : 10;
  //  std::cout << "nLayers: " << nLayers << std::endl;
  const char p0 = 'e'; // incident particle
  const double E0 = 3e3;
  const double mu0 = 0.999;

  std::vector<std::shared_ptr<Material>> mat;

  // tests
  if (argc==3) { // test nLayers
    //    std::cout << "test" << std::endl;
  }

  for (size_t i=0; i<nLayers; ++i)
    mat.push_back(W);

  auto sdef = mat[0]->getSDEF();
  sdef->Fill(E0, mu0);


  auto solver = std::make_shared<Solver>(p0, sdef, mat);
  solver->run(1);
  solver->save("res.root");

  static std::set<char> ftd {'n', 'p', 'e', '|'};
  for (auto p : ftd)
    std::cout << p << " " << solver->getDose(p) << "\t";

  std::cerr << "total: " << solver->getDose() << std::endl;

  return 0;
}

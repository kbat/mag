#include <iostream>
#include <boost/algorithm/string/replace.hpp>
#include <sys/ioctl.h>

#include "SolverArguments.h"


namespace std
{
  std::ostream& operator<<(std::ostream &os, const std::vector<std::string> &vec)
  {
    for (auto item : vec)
      os << item << " ";
    return os;
  }
}

SolverArguments::SolverArguments(int ac, const char **av) :
  argc(ac), argv(av), help(false)
{
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  try {
    //  options(argc, argv);
  po::options_description generic("Generic options", w.ws_col);
    generic.add_options()
      ("help,h", "Show this help message and exit.")
      ("test",  po::value<std::vector<size_t> >()->multitoken(),
       "Test number followed by number of layers to run")
      ("mat", "Print supported materials")
      ("sdef",  po::value<std::vector<std::string> >()->multitoken()->
       default_value(std::vector<std::string>{"e","3e3","0.999"}),
       "Source definition: PAR ERG DIR. PAR must be one of the particles ID "
       "for which the transport matrices are calculated, ERG is in MeV, DIR must be within (0,1)")
      ("layers", po::value<std::vector<std::string> >()->multitoken(),
       "Description of layers, e.g. 10 Concrete 4 Poly");

    po::positional_options_description pos;
    pos.add("layers", -1);
    //    pos.add("mat", 0);

    po::options_description all_options("Usage: gam-solve [options] [layers]");
    all_options.add(generic);

    //    po::store(po::parse_command_line(argc, argv, desc), vm);
    auto parsed = po::command_line_parser(argc, argv).options(all_options).positional(pos)
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

    if (!vm.count("mat") && !vm.count("layers"))
      {
	std::cerr << "gam-solve: list of layers must be specified" << std::endl;
	help = true;
      }
    if (help || vm.count("help"))
      {
        help = true;
        std::stringstream stream;
        stream << all_options;
        std::string helpMsg = stream.str();
        boost::algorithm::replace_all(helpMsg, "--", "-");
	//	boost::algorithm::replace_all(helpMsg, "-sdef arg", " sdef    ");
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

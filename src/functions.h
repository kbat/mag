#ifndef functions_h
#define functions_h

bool is_number(const std::string&);
void print_materials(const std::set<std::shared_ptr<Material> >&);
void set_sdef(const std::vector<std::string>&,
	      std::shared_ptr<TH2D>&,
	      std::map<char, std::shared_ptr<TH2D>>&);
bool check_layers(std::vector<std::shared_ptr<Material>>&);

#endif

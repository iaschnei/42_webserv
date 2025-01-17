#include "../../includes/ParsingData.hpp"

ParsingData::ParsingData(){
}

ParsingData::ParsingData(const ParsingData &copy)
{
	(void) copy;
	return ;
}

ParsingData::~ParsingData(){
}

ParsingData	&ParsingData::operator=(const ParsingData &copy)
{
	(void) copy;
	return (*this);
}

// Checks that the config file exists and that we can read it
// Using first argument or default path
// Then calls 'check_server_blocks()'
int ParsingData::config_parsing(int ac, char **av) {

	std::string path;

	if (ac > 2) {
		print_error(0, 0);
		return (0);
	}

	if (ac == 2) {
		path = av[1];

		if (path.length() < 4) {
			print_error(1, 0);
			return (0);
		}

		int path_size = path.size();
		if (path[path_size - 4] != '.' || path[path_size - 3] != 'c'
		|| path[path_size - 2] != 'n' || path[path_size - 1] != 'f') {
			print_error(1, 0);
			return (0);
		}
	}
	else {
		path = "./default_config.cnf";
	}

	std::ifstream fs;

	fs.open(path.c_str(), std::fstream::in);
	if (fs.fail()) {
		print_error(2, 0);
		return (0);
	}

	fs.peek();
	if (fs.eof()) {
		print_error(3, 0);
		return (0);
	}

	if (!check_server_blocks(&fs)) {
		return (0);
	}

	return (1);
}

// Checks that we only have closed server blocks or comments
// Then calls 'fill_data()'
int ParsingData::check_server_blocks(std::ifstream *fs) {

	std::string 				line;
	int 						line_num = 0;
	int 						server_count = 0;

	while (!fs->eof()) {

		std::getline(*fs, line);
		line_num++;

		if (fs->eof()) {
			break ;
		}

		if (line[0] == '#') {
			continue ;
		}

		if (line == "server {") {

			server_count++;
			this->servers_data_vec.resize(server_count);
			std::getline(*fs, line);
			line_num ++;
			while (line != "}") {

				if (fs->eof()) {
					print_error(4, line_num);
					return (0);
				}

				std::getline(*fs, line);
				line_num ++;
			}
		}
		else if (line.length() != 0) {
			print_error(4, line_num);
			return (0);
		}
	}

	fs->clear();
	fs->seekg(0);

	if (server_count == 0) {
		return (0);
	}

	if (!fill_data(fs)) {
		return (0);
	}

	fs->close();

	return (1);
}

// Will look for known keywords in 'known_keyword()'
// If they exist, check that the argument following is correct in 'keyword_argument_check()'
// That last function will then fill our data accordingly
int  ParsingData::fill_data(std::ifstream *fs) {

	std::string line;
	int 		line_num =  0;
	std::string	keyword;
	int 		space_pos;
	int 		server_index = 0;
	int			route_index;

	while (true) {

		route_index = 0;
		if (line == "server {") {
			this->servers_data_vec[server_index].server_name.append(1, server_index + '0');

			this->servers_data_vec[server_index].port_number = 0;
			this->servers_data_vec[server_index].max_client_body_size = 1000000;

			std::streampos savePos = fs->tellg();
			fs->seekg(savePos);
			this->servers_data_vec[server_index].routes_vec.clear();
			this->servers_data_vec[server_index].routes_vec.resize(count_location(fs));
			fs->seekg(savePos);

			while (line != "}") {
				std::getline(*fs, line);
				line_num ++;

				if (line == "}") {
					// std::cerr << "found '}' at line " << line_num << std::endl;
					break ;
				}

				// check that we always start with a tab
				if (line[0] != 9) {
					std::cerr << line << std::endl;
					print_error(4, line_num);
					return (0);
				}

				// find the delimitating space, if it isn't there, yikes
				space_pos = 1;

				while (line[space_pos] && line[space_pos] != ' ') {
					space_pos ++;
				}
				if (!line[space_pos]) {
					print_error(4, line_num);
					return (0);
				}

				keyword = line.substr(1, space_pos - 1);

				if (keyword == "location") {
					if (!add_a_route(fs, line, space_pos, server_index, route_index, &line_num)) {
						print_error(4, line_num);
						return (0);
					}
					route_index++;
				}
				else if (known_keyword(keyword)) {

					if (!keyword_argument_check(keyword, line, space_pos, server_index)) {
						print_error(4, line_num);
						return (0);
					}
				}
				else {
					print_error(5, line_num);
					return (0);
				}
			}

			if (this->servers_data_vec[server_index].hostname.empty()  ||
				this->servers_data_vec[server_index].port_number == 0  ||
				this->servers_data_vec[server_index].root_path.empty() ||
				this->servers_data_vec[server_index].index_filename.empty()) {
				print_error(6, line_num);
				return (0);
			}

			server_index ++;
		}

		std::getline(*fs, line);
		line_num ++;
		if (fs->eof()) {
			break ;
		}
	}

	return (1);
}

int ParsingData::count_location(std::ifstream *fs) {

	std::string line;
	int 		location_count = 0;

	while (true) {

		std::getline(*fs, line);

		if (fs->eof()) {
			break ;
		}

		if (line == "}") {
			break ;
		}

		if (line.find("location") != line.npos) {
			location_count++;
		}
	}

	return (location_count);
}

// Checks that the keyword passed in is a parameter we know
// Add a keyword to the list to add a known parameter
int ParsingData::known_keyword(std::string keyword) {

	std::vector<std::string>	compare_list;

	compare_list.push_back("host");
	compare_list.push_back("listen");
	compare_list.push_back("root");
	compare_list.push_back("index");
	compare_list.push_back("server_name");
	compare_list.push_back("max_client_body_size");
	compare_list.push_back("error_page");
	compare_list.push_back("allowed_methods");

	for (unsigned long i = 0; i < compare_list.size(); i++) {
		if (keyword == compare_list[i]) {
			return (1);
		}
	}

	return (0);
}

// Checks that the argument after the keyword is correct (string, unsigned int, ...)
// If it is, add it to the correct class
int ParsingData::keyword_argument_check(std::string keyword, std::string line, int space_pos, int server_index) {

	if (!line.empty() && line[line.length() - 1] != ';') {
		return (0);
	}

	std::string argument = line.substr(space_pos + 1, line.length() - space_pos - 2);

	if (keyword == "host") {

		if (!this->servers_data_vec[server_index].hostname.empty()) {
			return (0);
		}

		if (argument == "localhost") {
			this->servers_data_vec[server_index].hostname = "127.0.0.1";
		}

		else {
			if (is_path_valid(argument)) {
				this->servers_data_vec[server_index].hostname = argument;
			}
			else {
				return (0);
			}
		}
	}
	else if (keyword == "listen") {

		if (this->servers_data_vec[server_index].port_number != 0) {
			return (0);
		}

		if (is_port_valid(argument)) {
			this->servers_data_vec[server_index].port_number = (unsigned int) std::atoi(argument.c_str());
		}
		else {
			return (0);
		}

	}
	else if (keyword == "root") {

		if (!this->servers_data_vec[server_index].root_path.empty()) {
			return (0);
		}

		if (is_path_valid(argument)) {
			this->servers_data_vec[server_index].root_path = argument;
		}
		else {
			return (0);
		}

	}
	else if (keyword == "index") {

		if (!this->servers_data_vec[server_index].index_filename.empty()) {
			return (0);
		}

		if (is_path_valid(argument)) {
			this->servers_data_vec[server_index].index_filename = argument;
		}
		else {
			return (0);
		}

	}
	else if (keyword == "server_name") {

		if (this->servers_data_vec[server_index].server_name[0] != server_index + '0') {
			return (0);
		}

		if (is_name_valid(argument)) {
			this->servers_data_vec[server_index].server_name = argument;
		}
		else {
			return (0);
		}
	}
	else if (keyword == "max_client_body_size") {
		if (this->servers_data_vec[server_index].max_client_body_size != 1000000) {
			return (0);
		}

		if (is_size_valid(argument)) {
			this->servers_data_vec[server_index].max_client_body_size = (unsigned int) std::atoi(argument.c_str());
		}
		else {
			return (0);
		}
	}
	else if (keyword == "error_page") {

		// Not checking for duplicates in this case, too many cases

		if (is_error_page_valid(argument)) {
			add_error_page_to_data(argument, server_index);
		}
		else {
			return (0);
		}
	}
	else if (keyword == "allowed_methods") {

		if (this->servers_data_vec[server_index].allowed_methods != 0) {
			return (0);
		}

		if (!is_method_valid(argument, server_index)) {
			return (0);
		}
	}

	return (1);
}

void ParsingData::add_error_page_to_data(std::string argument, int server_index) {

	std::string path = argument.substr(4, argument.size());

	this->servers_data_vec[server_index].custom_error_pages.insert(std::make_pair(std::atoi(argument.substr(0,3).c_str()), path));
}

// The code is becoming a bit unclear going into subclasses of subclasses
// But basically this function will add a route to the route vector of a server_data
// It will also check that everything is written as expected (see default config file for details)
// If any error occurs return (0), else return (1)
int ParsingData::add_a_route(std::ifstream *fs, std::string line, int space_pos, int server_index, int route_index, int *line_num) {

	if (!line.empty() && line[line.length() - 1] != '{') {
		return (0);
	}

	std::string location_path = line.substr(space_pos + 1, line.length() - space_pos - 2);

	if (!is_path_valid(location_path)) {
		return (0);
	}
	else {
		this->servers_data_vec[server_index].routes_vec[route_index].location_name = location_path;
	}

	this->servers_data_vec[server_index].routes_vec[route_index].autoindex = false;
	this->servers_data_vec[server_index].routes_vec[route_index].allowed_methods = 0;

	std::string keyword;

	while (true) {

		std::getline(*fs, line);
		(*line_num)++;

		if (fs->eof()) {
			return (0);
		}

		if (line == "	}") {
			break ;
		}

		if (line[0] != 9 || line[1] != 9) {
			return (0);
		}

		space_pos = 2;
		while (line[space_pos] && line[space_pos] != ' ') {
			space_pos++;
		}
		if (!line[space_pos]) {
			return (0);
		}

		keyword = line.substr(2, space_pos - 2);

		if (known_route_keyword(keyword)) {

			if (!check_route_keyword(keyword, line, space_pos, server_index, route_index)) {
				return (0);
			}
		}
		else {
			print_error(5, *line_num);
			return (0);
		}

	}

	return (1);
}

int ParsingData::known_route_keyword(std::string keyword) {

	std::vector<std::string>	compare_list;

	compare_list.push_back("allowed_methods");
	compare_list.push_back("autoindex");
	compare_list.push_back("root_folder");
	compare_list.push_back("index");
	compare_list.push_back("cgi");
	compare_list.push_back("redirect");

	for (unsigned long i = 0; i < compare_list.size(); i++) {
		if (keyword == compare_list[i]) {
			return (1);
		}
	}

	return (0);
}

int ParsingData::check_route_keyword(std::string keyword, std::string line, int space_pos, int server_index, int route_index) {

	if (!line.empty() && line[line.length() - 1] != ';') {
		return (0);
	}

	std::string argument = line.substr(space_pos + 1, line.length() - space_pos - 2);

	if (keyword == "allowed_methods") {

		if (this->servers_data_vec[server_index].routes_vec[route_index].allowed_methods != 0) {
			return (0);
		}

		if (!is_method_valid(argument, server_index, route_index)) {
			return (0);
		}
	}
	else if (keyword == "autoindex") {

		if (argument == "on") {
			this->servers_data_vec[server_index].routes_vec[route_index].autoindex = true;
		}
		else if (argument == "off") {
			this->servers_data_vec[server_index].routes_vec[route_index].autoindex = false;
		}
		else {
			return (0);
		}
	}
	else if (keyword == "root_folder") {

		if (!this->servers_data_vec[server_index].routes_vec[route_index].root_folder.empty()) {
			return (0);
		}

		if (is_path_valid(argument)) {
			this->servers_data_vec[server_index].routes_vec[route_index].root_folder = argument;
		}
		else {
			return (0);
		}
	}
	else if (keyword == "index") {

		if (!this->servers_data_vec[server_index].routes_vec[route_index].index_filename.empty()) {
			return (0);
		}

		if (is_path_valid(argument)) {
			this->servers_data_vec[server_index].routes_vec[route_index].index_filename = argument;
		}
		else {
			return (0);
		}
	}
	else if (keyword == "cgi") {

		if (is_cgi_valid(argument)) {
			add_cgi_to_route(argument, server_index, route_index);
		}
		else {
			return (0);
		}
	}
	else if (keyword == "redirect") {

		if (!this->servers_data_vec[server_index].routes_vec[route_index].redirect_path.empty()) {
			return (0);
		}

		if (is_path_valid(argument)) {
			this->servers_data_vec[server_index].routes_vec[route_index].redirect_path = argument;
		}
		else {
			return (0);
		}
	}

	return (1);
}

void ParsingData::add_cgi_to_route(std::string argument, int server_index, int route_index) {

	int space_pos = 0;

	while (argument[space_pos] && argument[space_pos] != ' ') {
		space_pos++;
	}

	std::string	file_ext = argument.substr(0, space_pos);
	std::string path = argument.substr(space_pos + 1, argument.length());

	this->servers_data_vec[server_index].routes_vec[route_index].used_cgis.insert(std::make_pair(file_ext, path));
}

bool ParsingData::is_method_valid(std::string method, int server_index, int route_index) {

	std::string section;
	std::istringstream ss(method);

	if (method.empty()) {
		return (false);
	}

	while (std::getline(ss, section, ' ')) {

		if (section == "GET") {
			this->servers_data_vec[server_index].routes_vec[route_index].allowed_methods += 1;
		}
		else if (section == "POST") {
			this->servers_data_vec[server_index].routes_vec[route_index].allowed_methods += 10;
		}
		else if (section == "DELETE") {
			this->servers_data_vec[server_index].routes_vec[route_index].allowed_methods += 100;
		}
		else {
			return (false);
		}
	}

	return (true);
}

bool ParsingData::is_method_valid(std::string method, int server_index) {

	std::string section;
	std::istringstream ss(method);

	if (method.empty()) {
		return (false);
	}

	while (std::getline(ss, section, ' ')) {

		if (section == "GET") {
			this->servers_data_vec[server_index].allowed_methods += 1;
		}
		else if (section == "POST") {
			this->servers_data_vec[server_index].allowed_methods += 10;
		}
		else if (section == "DELETE") {
			this->servers_data_vec[server_index].allowed_methods += 100;
		}
		else {
			return (false);
		}
	}

	return (true);
}


/*  PRINT_ERROR
/   Do not change first and last line of function, they handle colors
/   To add an error message : add an if statement then output your message in std::cerr WITHOUT endline
*/
void	ParsingData::print_error(int error_code, int line_num) {
	std::cerr << RED << "YIKES!" << std::endl;

	if (error_code == 0) {
		std::cerr << "Invalid number of arguments, please input a single filename or none to use default config.";
	}
	else if (error_code == 1) {
		std::cerr << "Invalid path to config file, please input a '.cnf' file.";
	}
	else if (error_code == 2) {
		std::cerr << "Unable to open file, please check permissions.";
	}
	else if (error_code == 3) {
		std::cerr << "Empty file.";
	}
	else if (error_code == 4) {
		std::cerr << "Invalid config near line " << line_num << ".";
	}
	else if (error_code == 5) {
		std::cerr << "Unknown keyword near line " << line_num << ".";
	}
	else if (error_code == 6) {
		std::cerr << "Missing mandatory configuration, aborting.";
	}

	std::cerr << RESET << std::endl;
}


void	ParsingData::assign_server_to_client(Request *request, Response	*response) {

	std::map<std::string, std::string>::iterator it = request->header_params.find("Host");
	if (it == request->header_params.end()) {
		request->status_code = 400;
		return ;
	}

	unsigned int port = std::atoi((it->second.substr(it->second.find(":", 0) + 1, it->second.npos)).c_str());

	for (size_t i = 0; i < this->servers_data_vec.size(); i++) {
		if (port == this->servers_data_vec[i].port_number) {
			response->assigned_serv = &this->servers_data_vec[i];
			return ;
		}
	}
	return ;
}

void	ParsingData::set_max_client_body_size(Request *request) {

	std::map<std::string, std::string>::iterator it = request->header_params.find("Host");
	if (it == request->header_params.end()) {
		request->status_code = 400;
		return ;
	}

	unsigned int port = std::atoi((it->second.substr(it->second.find(":", 0) + 1, it->second.npos)).c_str());

	for (size_t i = 0; i < this->servers_data_vec.size(); i++) {
		if (port == this->servers_data_vec[i].port_number) {
			request->max_client_body_size = this->servers_data_vec[i].max_client_body_size;
			return ;
		}
	}
	return ;
}


#ifndef PARSING_DATA_HPP
# define PARSING_DATA_HPP

# include <string>
# include <vector>
# include <map>
# include <sstream>
# include <iostream>
# include <istream>
# include <fstream>
# include "Server_data.hpp"
# include "utils.hpp"
# include "Colors.hpp"
# include "Response.hpp"
# include "Request.hpp"

class ParsingData {

public:

	ParsingData();
	~ParsingData();

	std::vector<Server_data>	servers_data_vec;

	int 	config_parsing(int ac, char **av);
	void	assign_server_to_client(Request	*request, Response *response);
	void	set_max_client_body_size(Request *request);

private:

	ParsingData(ParsingData const &copy);
	ParsingData &operator=(ParsingData const &copy);

	int 	check_server_blocks(std::ifstream *fs);
	int 	fill_data(std::ifstream *fs);
	int 	count_location(std::ifstream *fs);
	int 	known_keyword(std::string keyword);
	int 	keyword_argument_check(std::string keyword, std::string line, int space_pos, int server_index);
	void	add_error_page_to_data(std::string argument, int server_index);
	int 	add_a_route(std::ifstream *fs, std::string line, int space_pos, int server_index, int route_index, int *line_num);
	int 	known_route_keyword(std::string keyword);
	int 	check_route_keyword(std::string keyword, std::string line, int space_pos, int server_index, int route_index);
	void 	add_cgi_to_route(std::string argument, int server_index, int route_index);
	void    print_error(int error_code, int line_num);
	bool 	is_method_valid(std::string method, int server_index, int route_index);
	bool 	is_method_valid(std::string method, int server_index);

};



#endif

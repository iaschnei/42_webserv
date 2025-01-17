#ifndef SERVER_DATA_HPP
# define SERVER_DATA_HPP

# include <string>
# include <map>
# include <vector>
# include "Route.hpp"

class Server_data {

public:
	Server_data();
	~Server_data();
	Server_data &operator=(const Server_data &rhs);
	Server_data(const Server_data &rhs);

// Mandatory

	std::string		hostname;
	unsigned int	port_number;
	std::string		root_path;
	std::string		index_filename;
	unsigned int	allowed_methods;

// Optional

	std::string 				server_name;
	unsigned int 				max_client_body_size;
	std::map<int, std::string>	custom_error_pages;

	// Routes

	std::vector<Route> routes_vec;

};

#endif
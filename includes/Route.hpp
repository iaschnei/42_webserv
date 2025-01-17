#ifndef ROUTE_HPP
# define ROUTE_HPP

# include <string>
# include <map>

class Route {

public:
	Route();
	~Route();
	Route(const Route &rhs);
	Route	&operator=(const Route &rhs);

	std::string 						location_name;
	// For allowed_methods: 1 for GET, 10 for POST, 100 for DELETE (111 for all three, 11 for GET and POST...)
	unsigned int						allowed_methods;
	bool		 						autoindex;
	std::string							root_folder;
	std::string							index_filename;
	std::string							upload_directory;
	std::map<std::string, std::string>	used_cgis;
	std::string							redirect_path;
};

#endif
#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <sstream>

bool 		is_ip_valid(std::string ip);
bool 		is_port_valid(std::string port);
bool 		is_path_valid(std::string path);
bool		is_name_valid(std::string name);
bool 		is_size_valid(std::string size);
bool 		is_error_page_valid(std::string argument);
bool		is_cgi_valid(std::string argument);
std::string getStatus(int code);
std::string determine_file_type(std::string	extension);

#endif

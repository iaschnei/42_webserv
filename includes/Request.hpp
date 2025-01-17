#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <sys/socket.h>
# include <netinet/in.h>
# include <string>
# include <map>
# include "constants.hpp"
# include "utils.hpp"
# include "Colors.hpp"
# include <iostream>
# include <cstdlib>
# include <cstdio>
# include <cstring>

class Request {

public:

	Request();
	Request(Request const & rhs);
	Request &operator=(Request const & rhs);
	~Request();

	int 				status_code;

	enum Method 		method_type;
	enum Status			current_status;

	std::string			header;
	std::string			body;

	std::string			method;
	std::string			uri;
	std::string			version;

	std::string			first_line_str;
	std::string			headers_str;
	std::string			body_str;

	std::string			query_str;

	size_t 				end_of_first_line;
	size_t 				end_of_header;
	size_t				expected_body_size;
	unsigned int 		max_client_body_size;

	bool				are_params_url_encoded;

	bool				done_parsing_first_line;
	bool				done_parsing_headers;
	bool				expected_body_size_set;
	bool				done_reading_body;

	std::map<std::string, std::string>	header_params;
	std::map<std::string, std::string>  query_params;

	void		read_request(int client_fd);
	int			read_and_parse_first_line(int client_fd);
	int			parse_first_line(void);
	int			read_and_parse_headers(int client_fd);
	int			parse_headers(void);
	int			read_body(int client_fd);
	int			read_one_buffer(int client_fd, std::string &dest);
	void		fill_query_params(void);
	char		decode_char(const std::string& str, size_t& pos);
	std::string	decode(const std::string& encoded);
};

std::ostream &	operator<<(std::ostream & os, Request *request);


#endif

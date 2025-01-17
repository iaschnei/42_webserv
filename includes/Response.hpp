#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <sys/socket.h>
# include <netinet/in.h>
# include <string>
# include <map>
# include "constants.hpp"
# include "utils.hpp"
# include "Colors.hpp"
# include <iostream>
# include <sstream>
# include "Request.hpp"
# include "Server_data.hpp"
# include <unistd.h>
# include <fstream>
# include <cstring>
# include <cstdlib>
# include <fcntl.h>
# include <dirent.h>
# include <sys/stat.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/wait.h>

class Response {

public:
	Response();
	Response(Response &rhs);
	~Response();

	Server_data							*assigned_serv;
	Route								*location;

	enum Method 						method_type;
	enum Status							current_status;
	enum MultipartMode					mode;

	int 								status_code;

	std::string							uri;
	std::string							full_path;

	std::string							content_type;

	std::string							response_header;
	std::stringstream 					response_body_stream;
	std::string							response_string;
	std::map<std::string, std::string>	request_query_params;
	std::string							request_query_str;

	std::string							multipart_boundary;
	std::string							multipart_current_filename;
	std::string							multipart_file_content;
	bool								skipped_first_multipart_boundary;
	bool								done_with_multipart;

	size_t								left_to_post;
	size_t								left_to_send;
	size_t								already_sent;

	bool								done_checking;
	bool								done_sending;
	bool								done_with_cgi;
	bool								sent_header;

	std::map<std::string, std::string>	request_header_params;
	std::string							request_body;
	std::stringstream					request_body_stream;
	bool								request_error;

	int									pipe_in_fd[2];
	int									pipe_out_fd[2];

//	---------------------------------------------------------------------------

	void 	handle_request(int client_fd);
	void	assemble_response_string(void);
	void 	send_response(int client_fd);
	int		execute_get(void);
	int		execute_delete(void);
	void	find_location(void);
	int		set_response_status_code_and_header(int status_code);
	int		check_access_rights(std::string full_path);
	int		try_ls_dir(std::string full_path);
	int		ls_dir(DIR *dirstream);
	int		get_file_content(std::string full_path);
	void	check_custom_pages();
	int		check_delete_rights(std::string full_path);
	int		try_delete_file(std::string full_path);
	int		execute_post(void);
	int		check_parent_folder_writing_rights(std::string full_path);
	int		check_if_multipart_possible(std::string full_path);
	int		try_write_to_file(std::string full_path);
	int		try_write_to_file(std::string full_path, std::string content);
	int		try_write_multipart_files(std::string full_path);
	int		handle_cgi(std::string python_path);
	int		get_cgi_content();
	void 	send_chunk(int client_fd);
};

#endif

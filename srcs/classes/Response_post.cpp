#include "../../includes/Response.hpp"
#include <string>

// Try to handle POST request and send the adequate response
// Returns the status code of the response
int		Response::execute_post(void) {

	if (!this->done_checking)
	{
		// If POST is not allowed in the location
		if (this->location && ((this->location->allowed_methods % 100 != 11) && (this->location->allowed_methods % 100 != 10)))
			return (this->status_code = 405, 405);
		else if (!this->location && ((this->assigned_serv->allowed_methods % 100 != 11) && (this->assigned_serv->allowed_methods % 100 != 10)))
			return (this->status_code = 405, 405);

		// Build endpoint path
		if (!this->location || this->location->root_folder.empty())
			this->full_path = this->assigned_serv->root_path + this->uri;
		else
			this->full_path = this->location->root_folder + this->uri.substr(this->uri.find(this->location->location_name) + this->location->location_name.length());
		std::cerr << BLUE << "Endpoint: " << full_path << RESET << std::endl;

		// If the endpoint is a CGI
		if (this->full_path.substr(this->full_path.length() - 3, 3) == ".py") 	// If the endpoint is a python script...
		{
			std::map<std::string, std::string>::const_iterator it = this->location->used_cgis.find(".py");
			if (it != this->location->used_cgis.end()) { 						// ...and if python cgis are supported
				if (!this->done_with_cgi) {
					this->status_code = this->handle_cgi(it->second);			// Execute it
					// if (close (pipe_fd[1]) == -1)
					// 	return (this->status_code = 500, 500);
					return (this->status_code);
				}
			}
		}

		// Store boundary of multipart if existing
		std::map<std::string, std::string>::iterator it = this->request_header_params.find("Content-Type");
		if (it != this->request_header_params.end()) {
			if (it->second.find("multipart/form-data") != it->second.npos) {
				this->multipart_boundary = "--" + it->second.substr(it->second.find("=") + 1, std::string::npos);
			}
		}

		if (this->multipart_boundary.empty())
		{
			// Accept files uploaded with no multipart only if filename is defined in a custom header "Filename: <filename>"
			std::map<std::string,std::string>::iterator it = this->request_header_params.find("Filename");
			if (it == this->request_header_params.end()) {
				return (this->status_code = 400, 400);
			}
			this->full_path += "/" + it->second;

			// If no multipart, check writing rights on the endpoint's parent folder
			if (this->check_parent_folder_writing_rights(this->full_path) == -1) {
				return (this->status_code);
			}
			this->left_to_post = this->request_body.size();
		}
		else
		{
			// If multipart, check if writing rights on the endpoint's parent folder
			if (this->check_if_multipart_possible(this->full_path) == -1) {
				return (this->status_code);
			}

		}
		this->done_checking = true;
	}

	if (this->multipart_boundary.empty())
		this->try_write_to_file(this->full_path);
	else
		this->try_write_multipart_files(this->full_path);

	return (this->status_code);
}

// Returns -1 if the user has not the sufficient writing rights for the file or folder at `full_path`
// (meaning write right on parent folder),
// setting the status code accordingly.
// Returns 0 on success.
int Response::check_parent_folder_writing_rights(std::string full_path) {

	std::string	parent_folder_path = full_path;

	// Build parent folder path
	while (!parent_folder_path.empty() && parent_folder_path[parent_folder_path.length() - 1] == '/')
		parent_folder_path.erase(parent_folder_path.length() - 1);
	parent_folder_path = parent_folder_path.substr(0, parent_folder_path.find_last_of('/')) + "/";
	std::cerr << BLUE << "Parent folder path: " << parent_folder_path << RESET << std::endl;

	// Check existence of parent folder
	if (access(parent_folder_path.c_str(), F_OK) == -1)
		return (this->status_code = 404, -1);

	// Check write right on parent folder
	if (access(parent_folder_path.c_str(), W_OK) == -1)
		return (this->status_code = 403, -1);

	return (0);
}

// Returns -1 if:
//	- the destination at `full_path` is not a folder,
//	- the user has not the writing rights for the folder,
// setting the status code accordingly.
// Returns 0 on success.
int	Response::check_if_multipart_possible(std::string full_path)
{
	// Check existence of destination
	if (access(full_path.c_str(), F_OK) == -1)
		return (this->status_code = 404, -1);

	// Check write right on destination
	if (access(full_path.c_str(), W_OK) == -1)
		return (this->status_code = 403, -1);

	// Check if the endpoint is a directory
	struct stat	endpoint_statbuf;
	if (stat(full_path.c_str(), &endpoint_statbuf) == -1)
		return (this->status_code = 500, 500);
	if (!S_ISDIR(endpoint_statbuf.st_mode))
		return (this->status_code = 403, 403);

	return (0);
}

// If file is a regular file with writing rights, append to it
// If it doesn't exist, create it
// Returns the status code set
int	Response::try_write_to_file(std::string full_path) {

	int file_fd;

	if (access(full_path.c_str(), F_OK) == -1) {
		file_fd = open(full_path.c_str(), O_CREAT | O_RDWR, 0644);
		if (file_fd == -1) {
			return (this->status_code = 500, 500);
		}
		return (close(file_fd), 100);
	}

	// Check if the endpoint is a file or a directory
	struct stat	endpoint_statbuf;
	if (stat(full_path.c_str(), &endpoint_statbuf) == -1) {
		return (this->status_code = 500, 500);
	}

	// If the endpoint is a directory
	if (S_ISDIR(endpoint_statbuf.st_mode))
		return (this->status_code = 403, 403);

	// If the endpoint is a regular file
	if (S_ISREG(endpoint_statbuf.st_mode))
	{
		int		send_size;
		int		written;

		file_fd = open(full_path.c_str(), O_WRONLY | O_APPEND);
		if (file_fd == -1) {
			return (this->status_code = 500, 500);
		}

		if (this->left_to_post > MAX_READ_SIZE) {
			send_size = MAX_READ_SIZE;
			this->left_to_post -= MAX_READ_SIZE;
		}
		else {
			send_size = this->left_to_post;
			this->status_code = 204;
		}

		written = write(file_fd, this->request_body.c_str() + this->already_sent, send_size);

		if (written == -1) {
			this->status_code = 500;
		}

		this->already_sent += written;

		if (close(file_fd) == -1) {
			this->status_code = 500;
		}
	}

	return (this->status_code);
}

// Steps:
//	1. skip the first boundary (once)
//	2. In the context of the outer loop (returning 100 till the end)
//		2.1 - Get the filename
//		2.2 - Skip the sub-header
//		2.3 - Store data into a string until the next boundary
//		2.4 - Try to write the string content to the appropriate file in the server
//		2.5 - Exit if reached last boundary (boundary + "--")
// Returns the status code set
int	Response::try_write_multipart_files(std::string full_path) {

	std::string	line;
	bool		parsing_multipart_file_content = false;
	bool		no_file_given = false;

	// Skip first line - once (first boundary)
	if (!this->skipped_first_multipart_boundary)
	{
		std::getline(this->request_body_stream, line);
		if (line.empty() || this->request_body_stream.eof())
			return (this->status_code = 400, 400);
		this->skipped_first_multipart_boundary = true;
	}

	if (this->mode == PARSE) {

		std::getline(this->request_body_stream, line);

		this->multipart_file_content.clear();
		this->multipart_current_filename.clear();

		// One complete loop for each file in multipart (the iteration on files is made on the upper main loop)
		while (!this->request_body_stream.eof() && line != this->multipart_boundary + "\r")
		{

			if (line == this->multipart_boundary + "--\r") { // Last boundary reached
				this->done_with_multipart = true;
				this->mode = WRITE;
				break ;
			}

			if (!no_file_given && this->multipart_current_filename.empty()) {
				size_t	filename_start_index = line.find("filename=\"") + 10;
				size_t	filename_length = line.substr(filename_start_index, std::string::npos).find("\"");
				// std::cerr << "Start index: " << filename_start_index << ", Length: " << filename_length << std::endl;
				if (filename_length == std::string::npos || filename_length == 0) {
					no_file_given = true;
				}
				this->multipart_current_filename = line.substr(filename_start_index, filename_length);
				// std::cerr << this->multipart_current_filename << std::endl;
			}

			if (!no_file_given && parsing_multipart_file_content) {
				this->multipart_file_content += line + "\n";
			}

			// Done skipping the file headers
			if (line == "\r") {
				parsing_multipart_file_content = true;
			}

			std::getline(this->request_body_stream, line);
		}

		if (this->request_body_stream.eof() && !this->done_with_multipart) {
			return (this->status_code = 400, 400);
		}

		this->multipart_file_content += "\0";

		this->mode = WRITE;
		this->left_to_post = this->multipart_file_content.size();
	}

	// For the file previously parsed
	if (this->mode == WRITE) {

		if (!this->multipart_current_filename.empty())
			try_write_to_file(full_path + "/" + this->multipart_current_filename, this->multipart_file_content);
		else
			this->status_code = 204;

		if (this->left_to_post < this->multipart_file_content.size() && this->multipart_file_content.size() > MAX_READ_SIZE) {
			this->multipart_file_content = this->multipart_file_content.substr(MAX_READ_SIZE, this->multipart_file_content.npos);
		}

		if (this->status_code == 204) {
			this->mode = PARSE;
			this->status_code = 100;
		}

		if (this->done_with_multipart && this->mode == PARSE) {
			this->status_code = 204;
		}
	}

	return (this->status_code);
}

// If file is a regular file with writing rights, append to it
// If it doesn't exist, create it
// Returns the status code set
int	Response::try_write_to_file(std::string full_path, std::string content) {

	int file_fd;
	// std::cerr << "Try writing to file at " << full_path << std::endl;

	if (access(full_path.c_str(), F_OK) == -1) {
		file_fd = open(full_path.c_str(), O_CREAT | O_RDWR, 0644);
		if (file_fd == -1) {
			return (this->status_code = 500, 500);
		}
		return (close(file_fd), 100);
	}

	// Check if the endpoint is a file or a directory
	struct stat	endpoint_statbuf;
	if (stat(full_path.c_str(), &endpoint_statbuf) == -1) {
		return (this->status_code = 500, 500);
	}

	// If the endpoint is a directory
	if (S_ISDIR(endpoint_statbuf.st_mode))
		return (this->status_code = 403, 403);

	// If the endpoint is a regular file
	if (S_ISREG(endpoint_statbuf.st_mode))
	{
		int		send_size;

		file_fd = open(full_path.c_str(), O_WRONLY | O_APPEND);
		if (file_fd == -1) {
			return (this->status_code = 500, 500);
		}

		if (this->left_to_post > MAX_READ_SIZE) {
			send_size = MAX_READ_SIZE;
			this->left_to_post -= MAX_READ_SIZE;
		}
		else {
			send_size = this->left_to_post;
			this->status_code = 204;
		}

		if (write(file_fd, content.c_str(), send_size) == -1) {
			this->status_code = 500;
		}

		if (close(file_fd) == -1) {
			this->status_code = 500;
		}

	}

	// std::cerr << this->status_code << std::endl;
	return (this->status_code);
}

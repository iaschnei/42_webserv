#include "../../includes/Response.hpp"
#include <string>

// Try to handle GET request and send the adequate response
// Returns the status code of the response
int		Response::execute_get(void) {

	// If GET is not allowed in the location
	if (this->location && this->location->allowed_methods % 10 != 1)
		return (this->status_code = 405, 405);
	else if (!this->location && this->assigned_serv->allowed_methods % 10 != 1)
		return (this->status_code = 405, 405);

	// Build endpoint path
	if (this->location && !this->location->redirect_path.empty()) {
		this->full_path = this->location->redirect_path;
		return (this->status_code = 301, 301);
	}
	else if (!this->location || this->location->root_folder.empty()) {
		this->full_path = this->assigned_serv->root_path + this->uri;
	}
	else {
		this->full_path = this->location->root_folder + this->uri.substr(this->uri.find(this->location->location_name) + this->location->location_name.length());
	}

	std::cerr << BLUE << "Endpoint: " << this->full_path << RESET << std::endl;

	// check access rights to the endpoint
	if (this->check_access_rights(this->full_path) == -1)
		return (this->status_code);

	// Check if the endpoint is a file or a directory
	struct stat	endpoint_statbuf;
	if (stat(this->full_path.c_str(), &endpoint_statbuf) == -1)
		return (this->status_code = 500, 500);

	// If the endpoint is a regular file
	if (S_ISREG(endpoint_statbuf.st_mode))
	{
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
		return (this->get_file_content(this->full_path));
	}

	// If the endpoint is a directory
	if (S_ISDIR(endpoint_statbuf.st_mode))
	{
		// Add trailing '/' if not already there
		if (!this->full_path.empty() && this->full_path[this->full_path.length() - 1] != '/')
			this->full_path += "/";

		// Add index filename, at a path determined by wether we are in a specific location or not
		if (!this->location)
			this->full_path += this->assigned_serv->index_filename;
		else
			this->full_path += this->location->index_filename;
		std::cerr << BLUE << "Endpoint after adding index filepath: " << this->full_path << RESET << std::endl;

		// check access rights to the new endpoint
		if (this->check_access_rights(this->full_path) == -1)
			return (this->status_code);

		// Check if the new endpoint is a file or a directory
		struct stat	new_endpoint_statbuf;
		if (stat(this->full_path.c_str(), &new_endpoint_statbuf) == -1)
			return (this->status_code = 500, 500);

		// If the endpoint is a regular file
		if (S_ISREG(new_endpoint_statbuf.st_mode))
			return (this->get_file_content(this->full_path));

		// If the endpoint is a directory
		if (S_ISDIR(new_endpoint_statbuf.st_mode))
			return (this->try_ls_dir(this->full_path));
	}
	return (this->status_code = 500, 500);
}

// If file opened successfully, fill response body with the content of the file,
// set status code and header to "OK"
// Returns the status code set
int		Response::get_file_content(std::string full_path) {

	std::ifstream	file_stream;

	file_stream.open(full_path.c_str());

	if (file_stream.fail())
		return (this->status_code = 500, 500);

	if (this->status_code == 100) {
		this->status_code = 200;
	}
	std::string	line;
	while (std::getline(file_stream, line)) {
		this->response_body_stream << line << "\n";
	}
	return (this->status_code);
}

// Try to list contents of the directory (if autoindex)
// Returns the status code set
int		Response::try_ls_dir(std::string full_path) {

	DIR* dirstream =  opendir(full_path.c_str());

	if (!dirstream) // return if not a directory or if opening directory stream failed
		return (this->status_code = 500, 500);

	if (!this->location->autoindex) { // return if autoindex not allowed
		if (closedir(dirstream) == -1)
			return (this->status_code = 500, 500);
		return (this->status_code = 403, 403);
	}

	// Otherwise, ls_dir
	return (this->ls_dir(dirstream));
}

// List contents of the directory and add it to the response body
// set status code and header to "OK" (except if probelm occured when closing dirstream)
// Returns the status code set
int		Response::ls_dir(DIR *dirstream)
{
	if (this->status_code == 100) {
		this->status_code = 200;
	}

	struct dirent *dir_struct = readdir(dirstream);
	while (dir_struct)
	{
		this->response_body_stream << dir_struct->d_name << "\n";
		dir_struct = readdir(dirstream);
	}

	if (closedir(dirstream) == -1) {
		this->response_body_stream.str("");
		this->response_body_stream.clear();
		this->response_header.clear();
		this->status_code = 500;
	}

	return (this->status_code);
}

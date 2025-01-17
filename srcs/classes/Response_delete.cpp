#include "../../includes/Response.hpp"
#include <string>

// Try to handle DELETE request and send the adequate response
// Returns the status code of the response
int		Response::execute_delete(void) {

	// If DELETE is not allowed in the location
	if (this->location && this->location->allowed_methods < 100)
		return (this->status_code = 405, 405);
	else if (!this->location && this->assigned_serv->allowed_methods < 100)
		return (this->status_code = 405, 405);

	// Build endpoint path
	if (!this->location || this->location->root_folder.empty())
		this->full_path = this->assigned_serv->root_path + this->uri;
	else
		this->full_path = this->location->root_folder + this->uri.substr(this->uri.find(this->location->location_name) + this->location->location_name.length());
	std::cerr << BLUE << "Endpoint: " << this->full_path << RESET << std::endl;

	// check delete rights on the endpoint
	if (this->check_delete_rights(this->full_path) == -1) {
		return (this->status_code);
	}

	// Try to delete the file at endpoint
	return (try_delete_file(this->full_path));

}

// Returns -1 if the user has not the sufficient deletion rights for the file or folder at `full_path`
// (meaning write and execute rights on parent folder),
// setting the status code accordingly.
// Returns 0 on success.
int Response::check_delete_rights(std::string full_path) {

	std::string	parent_folder_path = full_path;

	// Build parent folder path
	while (!parent_folder_path.empty() && parent_folder_path[parent_folder_path.length() - 1] == '/')
		parent_folder_path.erase(parent_folder_path.length() - 1);
	parent_folder_path = parent_folder_path.substr(0, parent_folder_path.find_last_of('/')) + "/";
	std::cerr << BLUE << "Parent folder path: " << parent_folder_path << RESET << std::endl;

	// Check existence of parent folder
	if (access(parent_folder_path.c_str(), F_OK) == -1)
		return (this->status_code = 404, -1);

	// Check write and execute rights on parent folder
	if (access(parent_folder_path.c_str(), W_OK) == -1 || access(parent_folder_path.c_str(), X_OK) == -1)
		return (this->status_code = 403, -1);

	return (0);
}

// If file is a regular file, delete it.
// set status code and header to "OK"
// Returns the status code set
int	Response::try_delete_file(std::string full_path) {

	// Check if the endpoint is a file or a directory
	struct stat	endpoint_statbuf;
	if (stat(full_path.c_str(), &endpoint_statbuf) == -1)
		return (this->status_code = 500, 500);

	// If the endpoint is a directory
	if (S_ISDIR(endpoint_statbuf.st_mode))
		return (this->status_code = 403, 403);

	// If the endpoint is a regular file
	if (S_ISREG(endpoint_statbuf.st_mode))
	{
		if (unlink(full_path.c_str()) == 0)
			return (this->status_code = 204, 204);
	}

	// If didn't work
	return (this->status_code = 500, 500);
}

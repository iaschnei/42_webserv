#include "../../includes/Response.hpp"
#include <string>

Response::Response() {

	this->method_type = UNKNOWN;
	this->current_status = READY;

	this->mode = PARSE;

	this->status_code = 0;

	this->left_to_send = 0;
	this->left_to_post = 0;
	this->already_sent = 0;

	this->done_checking = false;
	this->skipped_first_multipart_boundary = false;
	this->done_sending = false;
	this->done_with_multipart = false;
	this->done_with_cgi = false;
	this->sent_header = false;

	this->location = NULL;
	this->assigned_serv = NULL;

	this->request_error = false;

	return ;
}

Response::Response(Response & rhs) {

	this->method_type = rhs.method_type;
	this->current_status = rhs.current_status;

	this->mode = rhs.mode;

	this->uri = rhs.uri;
	this->full_path = rhs.full_path;

	this->status_code = rhs.status_code;
	this->response_header = rhs.response_header;
	std::string temp = this->response_body_stream.str();
    rhs.response_body_stream.str(temp);
	this->response_string = rhs.response_string;
	this->already_sent = rhs.already_sent;
	this->multipart_boundary = rhs.multipart_boundary;
	this->multipart_current_filename = rhs.multipart_current_filename;
	this->multipart_file_content = rhs.multipart_file_content;

	this->request_header_params = rhs.request_header_params;
	this->request_body = rhs.request_body;
	std::string temp2 = this->request_body_stream.str();
    rhs.request_body_stream.str(temp);
	this->request_query_params = rhs.request_query_params;
	this->request_query_str = rhs.request_query_str;

	this->left_to_post = rhs.left_to_post;
	this->request_error = rhs.request_error;

	this->done_checking = rhs.done_checking;
	this->skipped_first_multipart_boundary = rhs.skipped_first_multipart_boundary;
	this->done_sending = rhs.done_sending;
	this->done_with_multipart = rhs.done_with_multipart;
	this->left_to_send = rhs.left_to_send;
	this->done_with_cgi = rhs.done_with_cgi;
	this->sent_header = rhs.sent_header;

	this->assigned_serv = rhs.assigned_serv;
	this->location = rhs.location;

	this->content_type = rhs.content_type;

}

Response::~Response() {
	return ;
}

// Processes the already parsed request and sends the appropriate response
void Response::handle_request(int client_fd) {

	this->find_location();

	// std::cerr << CYAN;
	// std::cerr << "Handling request for client number " << client_fd << std::endl;
	// std::cerr << " --Status code: " << this->status_code << std::endl;
	// std::cerr << " --Method type: " << this->method_type << std::endl;
	// std::cerr << RESET;

	if (this->request_error == true && this->status_code != 100) {
		this->assemble_response_string();
	}

	// If we're done processing the request or we encountered an error, send a response,
	// either in one block by default or in chunks for cgis,
	// but always called in parts of MAX_READ_SIZE bytes by the upper main loop
	if (!this->done_with_cgi && this->status_code != 100) {
		this->send_response(client_fd);
		if (this->done_sending)
			this->current_status = COMPLETE;
		return ;
	}
	else if (this->done_with_cgi) {
		this->get_cgi_content();
		this->send_chunk(client_fd);
		if (this->done_sending)
			this->current_status = COMPLETE;
		return ;
	}

	// Process the request
	// (called in parts of MAX_READ_SIZE bytes by the upper main loop if the method is POST)
	switch(this->method_type) {

		// GET: All processing done in one call per request
		case GET:
			this->execute_get();
			this->assemble_response_string();
			break;

		// POST: Processing done by parts of MAX_READ_SIZE bytes of the request body
		// => Called several times by the upper main loop until the request body is fully exhausted
		case POST:
			this->execute_post();
			this->assemble_response_string();
			break;

		// DELETE: All processing done in one call per request
		case DELETE:
			this->execute_delete();
			this->assemble_response_string();
			break;

		case UNKNOWN:
			break;
	}

	return ;
}

// Try to find a specific location define in the configuration file based on request URI
// if location is not found this->location will be NULL
// if location is found this->location points to it
void	Response::find_location(void) {

	if (!assigned_serv)
		return ;

	for (size_t i = 0; i < this->assigned_serv->routes_vec.size(); i++) {
		if (this->assigned_serv->routes_vec[i].location_name == this->uri.substr(0, this->assigned_serv->routes_vec[i].location_name.length())) {
			this->location = &this->assigned_serv->routes_vec[i];
		}
	}
	return ;
}

// Assemble the final response string
void Response::assemble_response_string(void) {

	// Check if there is a custom page set up for the given status_code
	// if so, overwrite the content of response_body_stream
	// ! Can modify the status code if an error happens during this operation
	if (this->assigned_serv)
		this->check_custom_pages();

	// After this point, the status code is written in response_header and should not change
	this->set_response_status_code_and_header(this->status_code);

	std::string			header_params_str;
	std::stringstream	header_params_sstream;

	if (this->status_code != 204) {
		header_params_sstream << "Content-Length: " << this->response_body_stream.str().size() << "\n";
		if (!this->full_path.empty()) {

			size_t extension_start = this->full_path.rfind(".");
			if (extension_start != std::string::npos)
				header_params_sstream << "Content-Type: " << determine_file_type(this->full_path.substr(extension_start, this->full_path.length() - extension_start)) << "\n";

			// std::cerr << RED << this->full_path.substr(extension_start, this->full_path.length() - extension_start) << RESET << std::endl;
		}
	}

	if (this->status_code == 301) {
		header_params_sstream << "Location: http://localhost:" << this->assigned_serv->port_number << "/" << this->location->redirect_path << "\n";
	}

	header_params_sstream << "\r\n";

	header_params_str = header_params_sstream.str();

	if (this->status_code != 204) {
		this->response_string = this->response_header + header_params_str + this->response_body_stream.str();
	}
	else {
		this->response_string = this->response_header + header_params_str;
	}
	this->left_to_send = this->response_string.size();
	// std::cerr << this->response_string << std::endl;
}

// Final step of our response process, send everything to our client
// If the response stream is too large, only send a part of it
// Once we're done, set 'dont_sending' to true, which will set the flag to 'COMPLETE' in 'handle_request()'
void 	Response::send_response(int client_fd) {

	size_t	send_size;
	int		send_flag;

	if (this->left_to_send > MAX_READ_SIZE) {
		send_size = MAX_READ_SIZE;
		this->left_to_send -= MAX_READ_SIZE;
		send_flag = MSG_MORE;
	}
	else {
		send_size = this->left_to_send;
		this->done_sending = true;
		send_flag = 0;
		std::cerr << GREEN << "Reached last send ! With a size of " << send_size << RESET << std::endl;
	}

	// std::cerr << BLUE << "Will attempt to send.." << RESET << std::endl;
	// std::cerr << BLUE << this->response_string.size() - already_sent << RESET << std::endl;

	if (send_size == 0)
		return ;
	int sent = send(client_fd, this->response_string.c_str() + already_sent, send_size, send_flag);
	if (sent == 0) {
		std::cerr << YELLOW << "WARNING: send() did not send any byte" << RESET << std::endl;
		return ;
	}
	else if (sent == -1) {
		std::cerr << RED << "ERROR: send() failed with errno set to: "  << errno <<  RESET << std::endl;
		return ;
	}
	this->already_sent += sent;

	std::cerr << MAGENTA << this->response_header<< RESET << std::endl;

	return ;
}

void	Response::check_custom_pages() {

	std::map<int, std::string>::const_iterator it = this->assigned_serv->custom_error_pages.find(this->status_code);

	std::string	error_page_path;

	if (it != this->assigned_serv->custom_error_pages.end()) {		// There is a custom error page
		this->response_body_stream.str("");
		this->response_body_stream.clear();
		this->response_header.clear();
		error_page_path = this->assigned_serv->root_path + "/" + it->second;
	}
	else {
		if (this->status_code == 404) {								// This is a special case, the subject requires a default error page
			error_page_path = "servers/errors/default_404.html";
		}
		else
			return ;												// There is no custom error page
	}

	this->full_path = error_page_path;

	if (access(error_page_path.c_str(), F_OK) == -1) {
		this->status_code = 404;
		return ;
	}

	if (access(error_page_path.c_str(), R_OK) == -1) {
		this->status_code = 403;
		return ;
	}

	std::ifstream	file_stream;

	file_stream.open(error_page_path.c_str());

	if (file_stream.fail()) {
		this->status_code = 500;
		return ;
	}

	std::string	line;
	while (std::getline(file_stream, line)) {
		this->response_body_stream << line << "\n";
	}
	this->response_string = this->response_body_stream.str();
	this->left_to_send = this->response_string.size();

	if (this->status_code == 100) {
		this->status_code = 200;
	}
}

// Set the status code and add it to our response_body_stream alongside the status message
// provided by 'getStatus()'. The '\r\n' indicate end of line
// One more '\r\n' is needed at end of header
int		Response::set_response_status_code_and_header(int status_code) {

	this->status_code = status_code;

	std::stringstream	header_ss;

	header_ss << "HTTP/1.1 " << status_code << " " << getStatus(this->status_code) << "\r\n";
	this->response_header = header_ss.str();
	return (status_code);
}

// Returns -1 if the user has not the sufficient access rights for the file or folder at `full_path`,
// setting the status code accordingly.
// Returns 0 on success.
int		Response::check_access_rights(std::string full_path) {

	if (access(full_path.c_str(), F_OK) == -1)
		return (this->status_code = 404, -1);

	if (access(full_path.c_str(), R_OK) == -1)
		return (this->status_code = 403, -1);

	return (0);
}

#include "../../includes/Request.hpp"
#include <string>

Request::Request(void) {

	this->status_code = 0;

	this->current_status = READY;
	this->method_type = UNKNOWN;

	this->expected_body_size = 0;
	this->max_client_body_size = 0;

	this->are_params_url_encoded = false;

	this->done_parsing_first_line = false;
	this->done_parsing_headers = false;
	this->done_reading_body = false;
}

Request::Request(Request const & rhs) {
	*this = rhs;
}

Request::~Request() {
	return ;
}

Request &Request::operator=(Request const & rhs) {

	this->status_code = rhs.status_code;

	this->method_type = rhs.method_type;
	this->current_status = rhs.current_status;

	this->header = rhs.header;
	this->body = rhs.body;

	this->method = rhs.method;
	this->uri = rhs.uri;
	this->version = rhs.version;

	this->first_line_str = rhs.first_line_str;
	this->headers_str = rhs.headers_str;
	this->body_str = rhs.body_str;

	this->query_str = rhs.query_str;

	this->end_of_first_line = rhs.end_of_first_line;
	this->end_of_header = rhs.end_of_header;
	this->expected_body_size = rhs.expected_body_size;
	this->max_client_body_size = rhs.max_client_body_size;

	this->are_params_url_encoded = rhs.are_params_url_encoded;

	this->done_parsing_first_line = rhs.done_parsing_first_line;
	this->done_parsing_headers = rhs.done_parsing_headers;
	this->expected_body_size_set = rhs.expected_body_size_set;
	this->done_reading_body = rhs.done_reading_body;

	this->header_params = rhs.header_params;
	this->query_params = rhs.query_params;

	return (*this);
}

// Reads the request and:
// - leaves `this->status_code` unset (0) while parsing is not finished and no error occurred
// - sets `this->status_code` to the appropriate status code upon error, to send it in response
// - sets `this->status_code` to 100 ("continue") if parsing finished without error
void Request::read_request(int client_fd) {

	// If an error occurred while parsing (a status code has been set for the error)
	if (this->status_code) {
		this->current_status = COMPLETE;
		return ;
	}
	// Parse first line
	else if (!this->done_parsing_first_line) {
		this->status_code = this->read_and_parse_first_line(client_fd);
	}
	// Parse headers
	else if (!this->done_parsing_headers) {
		this->status_code = this->read_and_parse_headers(client_fd);
	}
	// Parse body (POST requests only)
	// Check at every iteration that we are not going over max_client_body_size if it's defined
	else if (this->method_type == POST && !this->done_reading_body) {
		this->status_code = this->read_body(client_fd);

		if (this->max_client_body_size > 0 && this->body_str.size() > this->max_client_body_size * 1024) {
			this->status_code = 413;
			this->current_status = COMPLETE;
		}
	}
	// Done parsing request
	else {
		std::cerr << GREEN << "Request parsing complete" << RESET << std::endl;
		this->status_code = 100;
		this->current_status = COMPLETE;
	}
}

// Reads first line by parts of MAX_READ_SIZE bytes, populating progressively `this->first_line_str`
// until the end of first line is found (the remainder is stored in `this->headers_str`),
// and then parses the first line infos (method, URI, version)
// Returns:
// - a status code upon error to be set and sent by response in upper main loop
// - 0 otherwise
int	Request::read_and_parse_first_line(int client_fd) {

	this->current_status = ON_HEADER;
	this->end_of_first_line = this->first_line_str.find("\r\n", 0);

	// Reading part, progressively populating `this->first_line_str` by parts of MAX_READ_SIZE bytes,
	// executed in the main upper loop until a CRLF is found
	if (this->end_of_first_line == std::string::npos) {
		return (this->read_one_buffer(client_fd, this->first_line_str));
	}

	// Parsing part, used once the CRLF is found
	/// - Split first line part and headers part of the string if `read()` has overlapped on headers
	if (this->first_line_str.length() > this->end_of_first_line + 2) {
		this->headers_str = this->first_line_str.substr(this->end_of_first_line + 2, std::string::npos);
		this->first_line_str = this->first_line_str.substr(0, end_of_first_line);
	}
	std::cerr << "Request first line:\n" + this->first_line_str + "\n" << std::endl;
	/// - Parse first line infos
	return (this->parse_first_line());
}

// Parses the first line infos (method, URI, version)
// Returns:
// - a status code upon error to be set and sent by response in upper main loop
// - 0 otherwise
int	Request::parse_first_line(void) {

	size_t	space_pos = this->first_line_str.find(" ", 0);

	if (space_pos == std::string::npos) {
		return (400);
	}

	// Parse method type
	this->method = this->first_line_str.substr(0, space_pos);
	if (this->method == "GET") {
		this->method_type = GET;
	}
	else if (this->method == "POST") {
		this->method_type = POST;
	}
	else if (this->method == "DELETE") {
		this->method_type = DELETE;
	}
	else {
		return (501);
	}

	space_pos = this->first_line_str.find(" ", space_pos + 1);
	if (space_pos == std::string::npos) {
		return (400);
	}

	// Parse URI
	this->uri = this->first_line_str.substr(this->method.length() + 1, space_pos - this->method.length() - 1);
	if (!is_path_valid(this->uri)) {
		return (400);
	}
	// std::cerr << this->uri << std::endl;

	// Parse version
	this->version = this->first_line_str.substr(this->uri.length() + 1 + this->method.length() + 1, this->end_of_first_line - (this->uri.length() + 1 + this->method.length() + 1));
	if (this->version != "HTTP/1.1" && this->version != "HTTP/1") {
		return (505);
	}

	// If we find query parameters in the uri parse them
	size_t	query_start = this->uri.find("?");

	if (query_start != this->uri.npos) {

		this->query_str = this->uri.substr(query_start + 1, this->uri.length() - query_start);
		this->uri = this->uri.substr(0, query_start);
	}

	this->done_parsing_first_line = true;
	return (0);
}

// Reads headers by parts of MAX_READ_SIZE bytes, populating progressively `this->headers_str`
// until the end of headers is found (the remainder is stored in `this->body_str`),
// and then parses the headers
// Returns:
// - a status code upon error to be set and sent by response in upper main loop
// - 0 otherwise
int	Request::read_and_parse_headers(int client_fd) {

	this->end_of_header = this->headers_str.find("\r\n\r\n", 0);

	// Reading part, progressively populating `this->headers_str` by parts of MAX_READ_SIZE bytes,
	// executed in the main upper loop until 2 consecutive CRLFs are found
	if (this->end_of_header == std::string::npos) {
		return (this->read_one_buffer(client_fd, this->headers_str));
	}

	// Parsing part, used once the 2 consecutive CRLFs are found
	// Split Headers and body part of the string if `read()` has overlapped on body
	if (this->headers_str.length() > this->end_of_header + 2) {
		this->body_str = this->headers_str.substr(this->end_of_header + 2, std::string::npos);
		this->headers_str = this->headers_str.substr(0, this->end_of_header + 2);
	}
	// std::cerr << "Request headers:\n" + this->headers_str + "\n" << std::endl;
	// Parse headers
	return (this->parse_headers());
}

// Parses the headers
// Returns:
// - a status code upon error to be set and sent by response in upper main loop
// - 0 otherwise
int	Request::parse_headers(void) {

	std::istringstream	ss(this->headers_str);
	std::string			line;
	std::string			key;
	size_t				colon_pos;
	std::string			value;
	size_t				end_pos;

	std::getline(ss, line);

	while (!line.empty() && line != "\r") {

		line.erase(line.length() - 1, std::string::npos);
		colon_pos = line.find(":", 0);
		key = line.substr(0, colon_pos);
		end_pos = line.find("\r", 0);
		value = line.substr(colon_pos + 1 + 1, end_pos);
		this->header_params.insert(std::make_pair(key, value));
		std::getline(ss, line);
	}

	this->done_parsing_headers = true;
	return (0);
}

// Reads body by parts of MAX_READ_SIZE bytes, populating progressively `this->body_str`
// until the end of content,
// Returns:
// - a status code upon error to be set and sent by response in upper main loop
// - 0 otherwise
int	Request::read_body(int client_fd) {

	// Check (only once) if the body size is specified in the header, if so we can use it to determine how much to recv
	// Otherwise, we need to look for an end of file
	if (this->current_status != ON_BODY) {

		this->body = this->body_str.erase(0, 2); // remove the "/r/n" at the beginning
		std::map<std::string, std::string>::const_iterator it = this->header_params.find("Content-Length");
		if (it != this->header_params.end()) {
			this->expected_body_size = std::atoi(it->second.c_str());
		}
		it = this->header_params.find("Content-type");
		if (it != this->header_params.end() && it->second == "application/x-www-form-urlencoded") {
			this->are_params_url_encoded = true;
		}
	}
	this->current_status = ON_BODY;

	// Reading, progressively populating `this->body_str` by parts of MAX_READ_SIZE bytes,
	// executed in the main upper loop until expected body size is reached or an EOF is found
	// std::cerr << "Expected body size: " << this->expected_body_size << ", body size: " << this->body_str.size() << std::endl;
	if (this->body_str.size() < this->expected_body_size ||
		(!this->expected_body_size && this->body_str.find(EOF) == std::string::npos)) {
		return (this->read_one_buffer(client_fd, this->body_str));
	}
	// std::cerr << "Request body:\n" + this->body_str + "\n" << std::endl;


	this->done_reading_body = true;
	return (0);
}

// Reads MAX_READ_SIZE bytes and append it to the dest string
// Returns:
// - a status code upon error to be set and sent by response in upper main loop
// - 0 otherwise
int	Request::read_one_buffer(int client_fd, std::string &dest) {

	char		buffer[MAX_READ_SIZE + 1];
	std::memset(buffer, 0, MAX_READ_SIZE + 1);
	std::string	buffer_str;

	int	bytes_read = recv(client_fd, buffer, MAX_READ_SIZE, 0);

	if (bytes_read == -1) {
		return (500);
	}
	else if (bytes_read == 0) {
		return (400);
	}

	buffer_str.assign(buffer, bytes_read); // specify explicitly the number of bytes to avoid early EOF interpretation on binary data
	dest += buffer_str;
	return (0);
}

std::ostream &	operator<<(std::ostream & os, std::map<std::string, std::string> const & map) {
	std::map<std::string, std::string>::const_iterator it;

	for (it = map.begin(); it != map.end(); ++it) {
		std::pair<std::string, std::string> pair = *it;
		os << "	- " << pair.first << " --> " << pair.second << std::endl;
	}
	return (os);
}

std::ostream &	operator<<(std::ostream & os, Request *request)
{
	os << YELLOW
	<< "Method: " << request->method
	<< "\nURI: " << request->uri
	<< "\nVersion: " << request->version
	<< "\nCode: " << request->status_code
	<< "\nHeaders" << request->header_params
	<< RESET << std::endl;
	return (os);
}


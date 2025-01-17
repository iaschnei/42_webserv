#include "../../includes/Server_data.hpp"

Server_data::Server_data(void) {
	this->allowed_methods = 0;
	return ;
}

Server_data::~Server_data(void) {
	return ;
}

Server_data::Server_data(const Server_data &rhs) {
	*this = rhs;
	return ;
}

Server_data &Server_data::operator=(const Server_data &rhs) {

	this->hostname = rhs.hostname;
	this->port_number = rhs.port_number;
	this->root_path = rhs.root_path;
	this->index_filename = rhs.index_filename;

	this->server_name = rhs.server_name;
	this->max_client_body_size = rhs.max_client_body_size;
	this->custom_error_pages = rhs.custom_error_pages;

	this->routes_vec = rhs.routes_vec;

	this->allowed_methods = rhs.allowed_methods;

	return (*this);
}

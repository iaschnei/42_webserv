#include "../../includes/Client.hpp"

Client::Client(void) {

	this->client_addr.sin_family = 0;
	this->client_addr.sin_port = 0;
	this->client_addr.sin_addr.s_addr = 0;
	this->client_addr_len = 0;
	this->client_socket = 0;

	this->on_response = false;

	this->request = new Request;
	this->response = new Response;
}

Client::Client(Client const & rhs) {
	this->client_addr.sin_family = rhs.client_addr.sin_family;
	this->client_addr.sin_port = rhs.client_addr.sin_port;
	this->client_addr.sin_addr.s_addr = rhs.client_addr.sin_addr.s_addr;
	this->client_addr_len = rhs.client_addr_len;
	this->client_socket = rhs.client_socket;

	this->on_response = rhs.on_response;

	this->request = new Request(*(rhs.request));
	this->response = new Response(*(rhs.response));
	return;
}

Client &Client::operator=(Client const & rhs)
{
	this->client_addr.sin_family = rhs.client_addr.sin_family;
	this->client_addr.sin_port = rhs.client_addr.sin_port;
	this->client_addr.sin_addr.s_addr = rhs.client_addr.sin_addr.s_addr;
	this->client_addr_len = rhs.client_addr_len;
	this->client_socket = rhs.client_socket;

	this->on_response = rhs.on_response;

	delete this->request;
	delete this->response;
	this->request = new Request(*(rhs.request));
	this->response = new Response(*(rhs.response));

	return (*this);
}

Client::~Client() {
	delete this->request;
	delete this->response;
	return ;
}

void	Client::gather_request_data(void) {

	this->response->method_type = this->request->method_type;
	this->response->status_code = this->request->status_code;
	this->response->uri = this->request->uri;
	this->response->request_body = this->request->body_str;
	this->response->request_header_params = this->request->header_params;
	this->response->request_body_stream << this->request->body_str;
	this->response->request_query_str = this->request->query_str;

	if (this->request->status_code != 100) {
		this->response->request_error = true;
	}

	this->on_response = true;
}

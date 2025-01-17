#include "../../includes/Server.hpp"


Server::Server(const Server_data &server_data)
{
	// create listing socket
	this->listening_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (this->listening_socket == -1)
		throw(ServerException());

	// bind socket to an address
	this->listening_socket_addr.sin_family = AF_INET;
	this->listening_socket_addr.sin_port = htons(server_data.port_number);
	this->listening_socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(this->listening_socket, (struct sockaddr *) &listening_socket_addr, sizeof(listening_socket_addr)) == -1)
		throw(ServerException());

	// make the socket listening
	if (listen(this->listening_socket, BACKLOG) == -1)
		throw(ServerException());

	return ;
}

Server::Server(const Server &rhs)
{

	this->listening_socket = rhs.listening_socket;
	this->listening_socket_addr = rhs.listening_socket_addr;
	this->epoll_event = rhs.epoll_event;
	return ;
}

Server::~Server()
{
	return ;
}

Server	&Server::operator=(const Server &rhs)
{
	(void) rhs;
	return (*this);
}

const char *	Server::ServerException::what() const throw()
{
	return (strerror(errno));
}

#include <string>
#include <iostream>
#include <istream>
#include <fstream>
#include <vector>
#include <map>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include "../../includes/GlobalData.hpp"
#include "../../includes/Colors.hpp"
#include "../../includes/webserv.hpp"

GlobalData::GlobalData(){
}

GlobalData::GlobalData(const GlobalData &rhs)
{
	*this = rhs;
	return ;
}

GlobalData::~GlobalData()
{
	close(this->epoll_fd);
	for (size_t i = 0; i < this->servers_vec.size(); i++)
		close(this->servers_vec[i].listening_socket);
}

GlobalData	&GlobalData::operator=(const GlobalData &rhs)
{
	this->servers_vec = rhs.servers_vec;
	this->clients_vec = rhs.clients_vec;
	this->epoll_fd = rhs.epoll_fd;
	return (*this);
}

void	GlobalData::clean_on_signal(int signal)
{
	(void) signal;
	throw SignalInterruptException();
}

const char *	GlobalData::SignalInterruptException::what() const throw()
{
	return ("Program interrupted by signal");
}

const char *	GlobalData::GlobalDataException::what() const throw()
{
	return (strerror(errno));
}

/*
* Create an epoll instance, and set its events options to watch for incoming connections on each server
* */
void	GlobalData::epoll_init(void) {

	this->epoll_fd = epoll_create(1);
	if (this->epoll_fd == -1)
		throw(GlobalDataException());
	for (unsigned long i = 0; i < this->servers_vec.size(); i ++) {
		this->servers_vec[i].epoll_event.events = EPOLLIN;
		this->servers_vec[i].epoll_event.data.fd = this->servers_vec[i].listening_socket;
		if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, this->servers_vec[i].listening_socket, &this->servers_vec[i].epoll_event) == -1)
			throw(GlobalDataException());
	}
}


// Search the client socket with the file descriptor `fd` in the server's vector of connected clients
// Returns its index in `clients_vec` if found, -1 otherwise
int	GlobalData::get_client_index(int fd) {

	for (size_t i = 0; i < this->clients_vec.size(); i++) {
		if (this->clients_vec[i].client_socket == fd)
			return (i);
	}
	return (-1);
}

int	GlobalData::is_listening_socket(int fd) {

	for (size_t i = 0; i < this->servers_vec.size(); i++) {
		if (this->servers_vec[i].listening_socket == fd) {
			return (1);
		}
	}
	return (0);
}

void	GlobalData::setup_client(int events_fd)
{
	Client	client;

	client.client_socket = accept(events_fd, (sockaddr *) &client.client_addr, &client.client_addr_len); // connect to a client
	if (client.client_socket == -1)
		throw GlobalData::GlobalDataException();

	client.epoll_event.events = EPOLLIN|EPOLLOUT;
	client.epoll_event.data.fd = client.client_socket;
	this->clients_vec.push_back(client);

	if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, client.client_socket, &client.epoll_event) == -1)
		throw GlobalData::GlobalDataException();

	std::cerr << "Client " << this->clients_vec.size() << " accepted" << std::endl; // debug
	// std::cerr << client.client_socket << std::endl; //debug
}

void	GlobalData::close_on_completion(Client *current_client, int events_fd) {

	if (current_client && current_client->request->current_status == COMPLETE && current_client->response->current_status == COMPLETE) {

		this->clients_vec.erase(this->clients_vec.begin() + this->get_client_index(events_fd));
		if (close(events_fd) == -1)
			throw GlobalData::GlobalDataException();
	}
}

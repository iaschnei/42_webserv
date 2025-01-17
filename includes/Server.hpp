#ifndef SERVER_HPP
# define SERVER_HPP
#include "Server_data.hpp"
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <unistd.h>
# include <errno.h>
# include <sys/epoll.h>
# include <string>
# include <iostream>
# define BACKLOG 1024
# define MAX_EVENTS 100

# include <arpa/inet.h>
# include <stdlib.h>
# include <string.h>


class Server {

public:

	Server(const Server_data &server_data);
	Server(const Server &rhs);
	~Server();
	Server	&operator=(const Server &rhs);

	Server	&launch(void);

	int					listening_socket;
	struct	sockaddr_in	listening_socket_addr;
	struct	epoll_event	epoll_event;

	class	ServerException : public std::exception
	{
		virtual const char * what() const throw();
	};

};

#endif

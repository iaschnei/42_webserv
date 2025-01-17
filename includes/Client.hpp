#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <sys/socket.h>
# include <netinet/in.h>
# include "Request.hpp"
# include "Response.hpp"
# include "Server_data.hpp"
# include <sys/epoll.h>

class Client {

public:

	Client();
	Client(Client const & rhs);
	Client &operator=(Client const & rhs);
	~Client();

	void	gather_request_data(void);

	Request				*request;
	Response			*response;

	struct	sockaddr_in	client_addr;
	socklen_t			client_addr_len;
	int					client_socket;
	struct epoll_event	epoll_event;

	bool				on_response;
};

#endif

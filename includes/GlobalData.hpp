#ifndef GLOBAL_DATA_HPP
# define GLOBAL_DATA_HPP

# include <vector>
# include <csignal>
# include "Server_data.hpp"
# include "Server.hpp"
# include "Client.hpp"
# include "utils.hpp"

class GlobalData {

private:

	GlobalData(const GlobalData &rhs);
	GlobalData	&operator=(const GlobalData &rhs);

public:

	GlobalData();
	~GlobalData();

	std::vector<Server>			servers_vec;
	std::vector<Client>			clients_vec;

	int 						epoll_fd;

	void		epoll_init();

	static void		clean_on_signal(int signal);

	int			get_client_index(int fd);
	int			is_listening_socket(int fd);
	void		setup_client(int events_fd);
	void		close_on_completion(Client *current_client, int events_fd);

	class	SignalInterruptException : public std::exception
	{
		virtual const char * what() const throw();
	};

	class	GlobalDataException : public std::exception
	{
		virtual const char * what() const throw();
	};

};

#endif

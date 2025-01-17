#include <csignal>
#include "includes/webserv.hpp"

int main (int ac, char **av) {

	GlobalData	*global_data = new GlobalData();
	ParsingData	*parsing_data = new ParsingData();

	try
	{
		// HANDLE SIGINT
		signal(SIGINT, GlobalData::clean_on_signal);

		// PARSE CONFIGURATION FILE
		if (!parsing_data->config_parsing(ac, av)) {
			return (delete parsing_data, delete global_data, 1);
		}
		display_config(parsing_data); // Debug

		// CREATE AND LAUNCH SERVERS
		// for each server block config, create a Server an launch it (create socket, bind and listen)
		for (unsigned long i = 0; i < parsing_data->servers_data_vec.size(); i++)
		{
			Server	server(parsing_data->servers_data_vec[i]);
			global_data->servers_vec.push_back(server);
			std::cerr << "Server " << i << " launched" << std::endl; // debug
		}

		// EPOLL INIT
		struct epoll_event events[MAX_EVENTS];
		std::memset(events, 0, MAX_EVENTS * sizeof(epoll_event));
		Client *current_client;
		int		nfds;
		global_data->epoll_init();

		// EPOLL WATCH LOOP
		while (true) {

			// Wait for new connections and/or data ready to be sent/received on existing connections
			// For each request, the loop will continue until all request data has been received and all response data has been sent.
			nfds = epoll_wait(global_data->epoll_fd, events, MAX_EVENTS, 1000000 * TIMEOUT_SECONDS);
			if (nfds == -1)
				return (delete parsing_data, delete global_data, errno);

			// Loop on all triggered fds
			for (int i = 0; i < nfds; i++)
			{
				current_client = NULL;

				// If the event is a new connection
				if (global_data->is_listening_socket(events[i].data.fd)) {
					global_data->setup_client(events[i].data.fd);
				}

				// If the event is triggered on an already connected client socket
				if (global_data->get_client_index(events[i].data.fd) != -1) {

					// Client identification
					current_client = &global_data->clients_vec[global_data->get_client_index(events[i].data.fd)];

					// Request parsing
					if (current_client->request->current_status != COMPLETE) {

						if (current_client->request->done_parsing_headers && current_client->request->method_type == POST) {
							parsing_data->set_max_client_body_size(current_client->request);
						}

						current_client->request->read_request(events[i].data.fd);

						if (current_client->request->current_status == COMPLETE) {
							std::cerr << current_client->request << std::endl;
							parsing_data->assign_server_to_client(current_client->request, current_client->response);
						}
					}
					// Request handling and response sending
					else if (current_client->response->current_status != COMPLETE) {

						if (!current_client->on_response) {
							current_client->gather_request_data();
						}

						current_client->response->handle_request(events[i].data.fd);
					}

					// Close connection if request has been fully handled
					global_data->close_on_completion(current_client, events[i].data.fd);
				}
			}
		}

		delete global_data;
		delete parsing_data;
	}

	catch(const std::exception& e)
	{
		std::cerr << RED << e.what() << RESET << std::endl;
		delete global_data;
		delete parsing_data;
	}
}


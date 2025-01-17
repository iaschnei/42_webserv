#include "../../includes/Response.hpp"
#include <string>

// Create a pipe in order to allow our cgi to communicate with us
// Fork the cgi process then wait for it to finish, setting done_with_cgi to true
int	Response::handle_cgi(std::string python_path) {

	// pipe_in for the program to receive the body of the request
	if (pipe(this->pipe_in_fd) == -1)
		return (this->status_code = 500, 500);

	// pipe_out to write the output of the program in the response body
	if (pipe(this->pipe_out_fd) == -1)
		return (this->status_code = 500, 500);

	pid_t	pid = fork();
	if (pid == 0) {

		// pipe_in for the program to receive the body of the request
		if (close(this->pipe_in_fd[1]) == -1)
			return (std::exit(500), 500);
		if (dup2(this->pipe_in_fd[0], STDIN_FILENO) == -1)
			return (std::exit(500), 500);
		if (close(this->pipe_in_fd[0]) == -1)
			return (std::exit(500), 500);

		// pipe_out to write the output of the program in the response body
		if (close(this->pipe_out_fd[0]) == -1)
			return (std::exit(500), 500);
		if (dup2(this->pipe_out_fd[1], STDOUT_FILENO) == -1)
			return (std::exit(500), 500);
		if (close(this->pipe_out_fd[1]) == -1)
			return (std::exit(500), 500);

		if (access(full_path.c_str(), X_OK) == -1 || access(python_path.c_str(), X_OK) == -1) // check execution rights on the script
			return (std::exit(500), 500);

		char *argv[] = {(char *) python_path.c_str(), (char *) full_path.c_str(), NULL};

		if (this->method_type == GET) {

			std::string	env_query_string = "QUERY_STRING=" + this->request_query_str;
			char *envp_get[] = {
				(char *)env_query_string.c_str(),
				(char *)"REQUEST_METHOD=GET",
				NULL
			};
			if (execve(python_path.c_str(), argv, envp_get) == -1) {
				return (std::exit(500), 500);
			}
		}
		else if (this->method_type == POST) {

			std::stringstream	env_content_len_stream;
			env_content_len_stream << "CONTENT_LENGTH=" << this->request_body.size();
			std::string	env_content_len = env_content_len_stream.str();

			char *envp_post[] = {
				(char *)"REQUEST_METHOD=POST",
				(char *)"CONTENT_TYPE=application/x-www-form-urlencoded",
				(char *)env_content_len.c_str(),
				NULL
			};
			if (execve(python_path.c_str(), argv, envp_post) == -1) {
				return (std::exit(500), 500);
			}
		}

	}
	else if (pid == -1) {
		return (this->status_code = 500, 500);
	}
	else {
		int return_status;

		if (close(this->pipe_in_fd[0]) == -1)
				return (this->status_code = 500, 500);
		std::cout <<this->request_body.c_str() << std::endl;
		if (write(pipe_in_fd[1], this->request_body.c_str(), this->request_body.size()) == -1)
				return (this->status_code = 500, 500);
		if (close(this->pipe_in_fd[1]) == -1)
			return (this->status_code = 500, 500);

		if (close(this->pipe_out_fd[1]) == -1)
			return (this->status_code = 500, 500);

		if (waitpid(pid, &return_status, 0) == -1)
			return (this->status_code = 500, 500);

		if (return_status == 500)
			return (this->status_code = 500, 500);

		this->done_with_cgi = true ;

		return (this->status_code = 200, 200);
	}

	return (this->status_code = 500, 500);
}

// Read the content our cgi printed in the pipe
// Then, for each read call, add it as a chunk in our response, preceded by it's size
int	Response::get_cgi_content() {

	this->response_body_stream.str(std::string());
	this->response_body_stream.clear();

	char	buffer[MAX_READ_SIZE + 1];

	std::memset(buffer, 0, MAX_READ_SIZE + 1);

	int bytes_read = read(pipe_out_fd[0], buffer, MAX_READ_SIZE);

	if (bytes_read == -1 || bytes_read == 0) {

		this->done_sending = true;

		this->response_body_stream << "0\r\n" << "\r\n";

		if (bytes_read == -1)
			return (this->status_code = 500, 500);

		if (close(this->pipe_out_fd[0]) == -1)
			return (this->status_code = 500, 500);

		return (this->status_code = 200, 200);
	}

	buffer[bytes_read] = 0;
	this->response_body_stream << std::hex << strlen(buffer) << "\r\n";
	this->response_body_stream << buffer << "\r\n";
	return (this->status_code = 200, 200);
}

// Send one chunk at a time
// send_size can only be MAX_READ_SIZE (+ the size of the header in the first case) long (actually checked by get_cgi_content)
void 	Response::send_chunk(int client_fd) {

	size_t	send_size;
	int		send_flag;

	if (!this->sent_header) {
		this->response_string = this->response_header + "Transfer-Encoding: chunked\nContent-type: text/html\n" + "\r\n" + this->response_body_stream.str();
		this->sent_header = true;
	}
	else {
		this->response_string = this->response_body_stream.str();
	}

	send_size = this->response_string.size();
	send_flag = 0;

	std::cerr << GREEN << this->response_string << RESET << std::endl;

	int sent = send(client_fd, this->response_string.c_str(), send_size, send_flag);
	if (sent == 0) {
		std::cerr << RED << "WARNING: send() did not send any byte" << RESET << std::endl;
	}
	else if (sent == -1) {
		std::cerr << RED << "ERROR: send() failed with errno set to " << errno << RESET << std::endl;
	}

	return ;
}

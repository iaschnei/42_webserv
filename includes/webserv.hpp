#ifndef WEBSERV_HPP
# define WEBSERV_HPP

// ---------------------------------------------------------------------------

# include <vector>
# include <map>
# include <utility>
# include <string>
# include <iostream>
# include <istream>
# include <fstream>
# include <cctype>
# include <cstdlib>
# include <cstring>
# include <sstream>
# include <sys/epoll.h>
# include "GlobalData.hpp"
# include "ParsingData.hpp"
# include "Server_data.hpp"
# include "Server.hpp"
# include "Colors.hpp"

// ---------------------------------------------------------------------------


int 	config_parsing(int ac, char **av, GlobalData *global_data);
void	display_config(ParsingData *parsing_data);

// ---------------------------------------------------------------------------

#endif

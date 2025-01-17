#include "../includes/webserv.hpp"
#include "../includes/Colors.hpp"

void	display_config(ParsingData *parsing_data) {

	unsigned long server_count = 0;

	std::cerr << CYAN;

	std::cerr << "---------- [Config display] ---------" << std::endl;
	std::cerr << "- Number of servers: " << parsing_data->servers_data_vec.size() << std::endl;
	std::cerr << "-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_" << std::endl;

	while (server_count < parsing_data->servers_data_vec.size()) {

		std::cerr << std::endl;

		std::cerr << "---------- [Server " << server_count << "] ----------" << std::endl;
		std::cerr << "- Server name      -> " <<  parsing_data->servers_data_vec[server_count].server_name << std::endl;
		std::cerr << "- Host ip          -> " <<  parsing_data->servers_data_vec[server_count].hostname << std::endl;
		std::cerr << "- Port             -> " <<  parsing_data->servers_data_vec[server_count].port_number << std::endl;
		std::cerr << "- Default root     -> " <<  parsing_data->servers_data_vec[server_count].root_path << std::endl;
		std::cerr << "- Default index    -> " <<  parsing_data->servers_data_vec[server_count].index_filename << std::endl;

		if (parsing_data->servers_data_vec[server_count].max_client_body_size != 0) {
			std::cerr << "- Max client size  -> " <<  parsing_data->servers_data_vec[server_count].max_client_body_size << std::endl;
		}

		std::map<int, std::string>::iterator it;
		for (it = parsing_data->servers_data_vec[server_count].custom_error_pages.begin(); it != parsing_data->servers_data_vec[server_count].custom_error_pages.end(); ++it) {
			std::pair<int, std::string> pair = *it;
			std::cerr << "- Error page       -> " << pair.first << " : " << pair.second << std::endl;
		}

		for (unsigned long i = 0; i < parsing_data->servers_data_vec[server_count].routes_vec.size(); i++) {
			std::cerr << std::endl;
			std::cerr << "- -  Location " << parsing_data->servers_data_vec[server_count].routes_vec[i].location_name << " - - " << std::endl;

			unsigned int method_value = parsing_data->servers_data_vec[server_count].routes_vec[i].allowed_methods;

			std::cerr << "-- Allowed methods -> ";
			if (method_value == 0) {
				std::cerr << "NONE";
			}
			if (method_value % 10 != 0) {
				std::cerr << "GET ";
			}
			if ((method_value / 10) % 100 % 10 != 0) {
				std::cerr << "POST ";
			}
			if ((method_value / 100) % 1000 != 0) {
				std::cerr << "DELETE";
			}
			std::cerr << std::endl;

			std::cerr << "-- Autoindex       -> " << parsing_data->servers_data_vec[server_count].routes_vec[i].autoindex << std::endl;
			std::cerr << "-- Root folder     -> " << parsing_data->servers_data_vec[server_count].routes_vec[i].root_folder<< std::endl;
			std::cerr << "-- Index file      -> " << parsing_data->servers_data_vec[server_count].routes_vec[i].index_filename << std::endl;
			std::cerr << "-- Upload dir      -> " << parsing_data->servers_data_vec[server_count].routes_vec[i].upload_directory << std::endl;

			std::map<std::string, std::string>::iterator it2;
			for (it2 = parsing_data->servers_data_vec[server_count].routes_vec[i].used_cgis.begin(); it2 != parsing_data->servers_data_vec[server_count].routes_vec[i].used_cgis.end(); ++it2) {
				std::pair<std::string, std::string> pair = *it2;
				std::cerr << "-- Used cgi        -> " << pair.first << " : " << pair.second << std::endl;
			}

			std::cerr << "- - - - - - - - - - - - -" << std::endl;
			std::cerr << std::endl;
		}

		std::cerr << std::endl;
		std::cerr << "-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_" << std::endl;
		server_count ++;
	}

	std::cerr << RESET;
}

bool is_ip_valid(std::string ip) {

	std::string section;
	std::istringstream ss(ip);
	int count = 0;

	if (ip.empty()) {
		return (false);
	}

	while (std::getline(ss, section, '.')) {
		try {
			int num = std::atoi(section.c_str());
			if (num < 0 || num > 255) {
				return (false);
			}
		} catch (...) {
			return (false);
		}
		count++;
	}

	if (count == 4)
		return (true);

	return (false);
}

bool is_port_valid(std::string port) {

	if (port.empty())
		return (false);

	int length = port.length();

	if (length > 5) {
		return (false);
	}

	for (int i = 0; i < length; ++i) {
		if (!std::isdigit(port[i]))
			return (false);
	}

	int integer = std::atoi(port.c_str());

	if (integer >= 1 && integer <= 65535)
		return (true);

	return (false);
}

bool is_path_valid(std::string path) {

	if (path.empty())
		return (false);

	int length = path.length();

	for (int i = 0; i < length; i++) {
		if (!std::isalnum(path[i]) && path[i] != '_' && path[i] != '.' && path[i] != '/' && path[i] != '-'
				&& path[i] != '%' && path[i] != '?' && path[i] != '=' && path[i] != '&') {
			return (false);
		}
	}

	return (true);
}

bool is_name_valid(std::string name) {

	if (name.empty())
		return (false);

	int length = name.length();

	for (int i = 0; i < length; i++) {
		if (!std::isalpha(name[i])) {
			return (false);
		}
	}

	return (true);
}

bool is_size_valid(std::string size) {

	if (size.empty())
		return (false);

	int length = size.length();

	if (length > 9) {
		return (false);
	}

	for (int i = 0; i < length; ++i) {
		if (!std::isdigit(size[i]))
			return (false);
	}

	int integer = std::atoi(size.c_str());

	if (integer >= 1 && integer <= 50000000)
		return (true);

	return (false);
}

bool is_error_page_valid(std::string argument) {

	if (argument.empty())
		return (false);

	int length = argument.length();

	for (int i = 0; i < 3; i++) {
		if (!std::isdigit(argument[i])) {
			return (false);
		}
	}

	int error_code = std::atoi(argument.substr(0,3).c_str());

	if (error_code < 400 || error_code > 599) {
		return (false);
	}

	if (argument[3] != ' ') {
		return (false);
	}

	if (!is_path_valid(argument.substr(4, length))) {
		return (false);
	}

	return (true);
}

bool is_cgi_valid(std::string argument) {

	if (argument.empty()) {
		return (false);
	}

	int space_pos = 0;

	while (argument[space_pos] && argument[space_pos] != ' ') {
		space_pos++;
	}
	if (!argument[space_pos]) {
		return (false);
	}

	std::string	file_ext = argument.substr(0, space_pos);
	std::string path = argument.substr(space_pos + 1, argument.length());

	if (file_ext.length() < 2) {
		return (false);
	}

	if (file_ext[0] != '.') {
		return (false);
	}

	for (unsigned long i = 1; i < file_ext.length(); i++) {
		if (!isalpha(file_ext[i])) {
			return (false);
		}
	}

	if (!is_path_valid(path)) {
		return (false);
	}

	return (true);
}

std::string getStatus(int code) {

	switch (code)
	{
		case 100:
			return ("Continue");
		case 200:
			return ("OK");
		case 201:
			return ("Created");
		case 202:
			return ("Accepted");
		case 203:
			return ("Non-Authoritative Information");
		case 204:
			return ("No Content");
		case 205:
			return ("Reset Content");
		case 206:
			return ("Partial Content");
		case 300:
			return ("Multiple Choices");
		case 301:
			return ("Moved Permanently");
		case 302:
			return ("Found");
		case 303:
			return ("See Other");
		case 304:
			return ("Not Modified");
		case 307:
			return ("Temporary Redirect");
		case 308:
			return ("Permanent Redirect");
		case 400:
			return ("Bad Request");
		case 401:
			return ("Unauthorized");
		case 402:
			return ("Payment Required");
		case 403:
			return ("Forbidden");
		case 404:
			return ("Not Found");
		case 405:
			return ("Method Not Allowed");
		case 406:
			return ("Not Acceptable");
		case 407:
			return ("Proxy Authentication Required");
		case 408:
			return ("Request Timeout");
		case 409:
			return ("Conflict");
		case 410:
			return ("Gone");
		case 411:
			return ("Length Required");
		case 412:
			return ("Precondition Failed");
		case 413:
			return ("Payload Too Large");
		case 414:
			return ("URI Too Long");
		case 415:
			return ("Unsupported Media Type");
		case 416:
			return ("Range Not Satisfiable");
		case 417:
			return ("Expectation Failed");
		case 418:
			return ("I'm a teapot");
		case 421:
			return ("Misdirected Request");
		case 425:
			return ("Too Early");
		case 426:
			return ("Upgrade Required");
		case 428:
			return ("Precondition Required");
		case 429:
			return ("Too Many Requests");
		case 431:
			return ("Request Header Fields Too Large");
		case 451:
			return ("Unavailable For Legal Reasons");
		case 500:
			return ("Internal Server Error");
		case 501:
			return ("Not Implemented");
		case 502:
			return ("Bad Gateway");
		case 503:
			return ("Service Unavailable");
		case 504:
			return ("Gateway Timeout");
		case 505:
			return ("HTTP Version Not Supported");
		case 506:
			return ("Variant Also Negotiates");
		case 510:
			return ("Not Extended");
		case 511:
			return ("Network Authentication Required");
		default:
			return ("Unknown");
	}
}

std::string determine_file_type(std::string	extension) {

	if (extension ==  ".aac")
		return ("audio/aac");
	else if (extension == ".abw")
		return ("application/x-abiword");
	else if (extension == ".apng")
		return ("image/apng");
	else if (extension == ".arc")
		return ("application/x-freearc");
	else if (extension == ".avif")
		return ("image/avif");
	else if (extension == ".avi")
		return ("video/x-msvideo");
	else if (extension == ".azw")
		return ("application/vnd.amazon.ebook");
	else if (extension == ".bin")
		return ("application/octet-stream");
	else if (extension == ".bmp")
		return ("image/bmp");
	else if (extension == ".bz")
		return ("application/x-bzip");
	else if (extension == ".bz2")
		return ("application/x-bzip2");
	else if (extension == ".cda")
		return ("application/x-cdf");
	else if (extension == ".csh")
		return ("application/x-csh");
	else if (extension == ".css")
		return ("text/css");
	else if (extension == ".csv")
		return ("text/csv");
	else if (extension == ".doc")
		return ("application/msword");
	else if (extension == ".docx")
		return ("application/vnd.openxmlformats-officedocument.wordprocessingml.document");
	else if (extension == ".eot")
		return ("application/vnd.ms-fontobject");
	else if (extension == ".epub")
		return ("application/epub+zip");
	else if (extension == ".gz")
		return ("application/gzip");
	else if (extension == ".gif")
		return ("image/gif");
	else if (extension == ".htm" || extension == ".html")
		return ("text/html");
	else if (extension == ".ico")
		return ("image/vnd.microsoft.icon");
	else if (extension == ".ics")
		return ("text/calendar");
	else if (extension == ".jar")
		return ("application/java-archive");
	else if (extension == ".jpg" || extension == ".jpeg")
		return ("image/jpeg");
	else if (extension == ".js")
		return ("text/javascript");
	else if (extension == ".json")
		return ("application/json");
	else if (extension == ".jsonld")
		return ("application/ld+json");
	else if (extension == ".mid" || extension == ".midi")
		return ("audio/midi");
	else if (extension == ".mjs")
		return ("text/javascript");
	else if (extension == ".mp3")
		return ("audio/mpeg");
	else if (extension == ".mp4")
		return ("video/mp4");
	else if (extension == ".mpeg")
		return ("video/mpeg");
	else if (extension == ".mpkg")
		return ("application/vnd.apple.installer+xml");
	else if (extension == ".odp")
		return ("application/vnd.oasis.opendocument.presentation");
	else if (extension == "ods")
		return ("application/vnd.oasis.opendocument.spreadsheet");
	else if (extension == "odt")
		return ("application/vnd.oasis.opendocument.text");
	else if (extension == "oga")
		return ("audio/ogg");
	else if (extension == "agx")
		return ("video/ogg");
	else if (extension == ".ogx")
		return ("application/ogg");
	else if (extension == ".opus")
		return ("audio/opus");
	else if (extension == ".otf")
		return ("font/otf");
	else if (extension == ".png")
		return ("image/png");
	else if (extension == ".pdf")
		return ("application/pdf");
	else if (extension == ".php")
		return ("application/x-httpd-php");
	else if (extension == ".ppt")
		return ("application/vvndms-powerpoint");
	else if (extension == ".pptx")
		return ("application/vnd.openxmlformats-officedocument.presentationml.presentation");
	else if (extension == ".rar")
		return ("application/vnd.rar");
	else if (extension == ".rtf")
		return ("application/rtf");
	else if (extension == ".sh")
		return ("application/x-sh");
	else if (extension == ".svg")
		return ("image/svg+xml");
	else if (extension == ".tar")
		return ("application/x-tar");
	else if (extension == ".tif" || extension == ".tiff")
		return ("image/tiff");
	else if (extension == ".ts")
		return ("video/mp2t");
	else if (extension == ".ttf")
		return ("font/ttf");
	else if (extension == ".txt")
		return ("text/plain");
	else if (extension == ".vsd")
		return ("application/vnd.visio");
	else if (extension == ".wav")
		return ("audio/wav");
	else if (extension == ".weba")
		return ("audio/webm");
	else if (extension == ".webm")
		return ("vdeo/webm");
	else if (extension == ".webp")
		return ("image/webp");
	else if (extension == ".woff")
		return ("font/woff");
	else if (extension == ".woff2")
		return ("font/woff2");
	else if (extension == ".xhtml")
		return ("application/xhtml+xml");
	else if (extension == ".xls")
		return ("application/vnd.ms-excel");
	else if (extension == ".xlsx")
		return ("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
	else if (extension == ".xml")
		return ("application/xml");
	else if (extension == ".xul")
		return ("application/vnd.mozilla.xul+xml");
	else if (extension == ".zip")
		return ("application/zip");
	else if (extension == ".7z")
		return ("application/x-7z-compressed");
	else
		return ("application/octet-stream");

	return (NULL);
}

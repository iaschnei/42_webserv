#include "../../includes/Route.hpp"

Route::Route(void) {
	return ;
}

Route::~Route(void) {
	return ;
}

Route::Route(const Route &copy) {
	*this = copy;
	return ;
}

Route &Route::operator=(const Route &rhs) {
	
	this->location_name = rhs.location_name;
	this->allowed_methods = rhs.allowed_methods;
	this->autoindex = rhs.autoindex;
	this->root_folder = rhs.root_folder;
	this->index_filename = rhs.index_filename;
	this->upload_directory = rhs.upload_directory;
	this->used_cgis = rhs.used_cgis;
	this->redirect_path = rhs.redirect_path;

	return (*this);
}
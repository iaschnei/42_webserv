# ---------------------------------------------------------------------------- #

SRCS		=	main.cpp \
				srcs/classes/Server_data.cpp \
				srcs/classes/Server.cpp \
				srcs/classes/Route.cpp \
				srcs/utils.cpp  \
				srcs/classes/Client.cpp \
				srcs/classes/GlobalData.cpp \
				srcs/classes/Request.cpp \
				srcs/classes/Response.cpp \
				srcs/classes/Response_get.cpp \
				srcs/classes/Response_delete.cpp \
				srcs/classes/Response_post.cpp \
				srcs/classes/Response_cgi.cpp \
				srcs/classes/ParsingData.cpp

# ---------------------------------------------------------------------------- #

OBJS		=	$(SRCS:.cpp=.o)

DEPS		=	$(SRCS:.cpp=.d)

C++			=	c++

STD			=	-std=c++98

INCL    	=	includes

FLAGS		=	-Wall -Wextra -Werror -MMD -MP

NAME		=	webserv

COLOR_GREEN	=	\033[1;32m
COLOR_RED	=	\033[1;31m
COLOR_BLUE	=	\033[3;36m
COLOR_PURP	=	\033[1;35m
COLOR_END	=	\033[0m

%.o: %.cpp
	@$(C++) $(FLAGS) $(STD) -I $(INCL) -g -c $< -o $@
	@echo "$(COLOR_BLUE) || Compiling ($<)...$(COLOR_END)"

all: $(NAME)

$(NAME): $(OBJS)
	@$(C++) $(OBJS) -o $(NAME)
	@echo "$(COLOR_GREEN) || Done !$(COLOR_END)"

clean:
	@rm -rf $(OBJS)
	@echo "$(COLOR_RED) || Cleaning files...$(COLOR_END)"

fclean: clean
	@rm -rf $(NAME) $(DEPS)
	@echo "$(COLOR_RED) || Cleaning library...$(COLOR_END)"

re: fclean all

.PHONY: re fclean all clean

-include $(DEPS)

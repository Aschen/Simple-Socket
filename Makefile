CXX	=	g++

SRCS	=	main_server.cpp Socket.cpp

SRCS_CLIENT	=	main_client.cpp Socket.cpp

CXXFLAGS=	-Wall -W -Wextra

OBJS	=	$(SRCS:.cpp=.o)

OBJS_CLIENT	=	$(SRCS_CLIENT:.cpp=.o)

NAME	=	server

NAME_CLIENT	=	client


all:		$(NAME) $(NAME_CLIENT)

$(NAME):	$(OBJS)
		$(CXX) -o $(NAME) $(OBJS)

$(NAME_CLIENT):	$(OBJS_CLIENT)
		$(CXX) -o $(NAME_CLIENT) $(OBJS_CLIENT)

clean:
		rm -f $(OBJS)
		rm -f $(OBJS_CLIENT)

fclean:		clean
		rm -f $(NAME)
		rm -f $(NAME_CLIENT)

re: fclean all

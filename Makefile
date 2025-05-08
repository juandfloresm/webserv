NAME 			:=	webserv

CC 				:=	c++

FLAGS 			:=	-Wall -Wextra -Werror -pedantic -std=c++98

SRC 			:=	main.cpp Connection.cpp Message.cpp Request.cpp Response.cpp

OBJ 			:=	$(SRC:.cpp=.o)

ARG				:=	./config/my.conf

all: $(NAME)

$(NAME):$(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

runner: re
	./$(NAME) $(ARG)

valgrind: re
	valgrind ./$(NAME) $(ARG)

sanitize: fclean $(OBJ)
	$(CC) $(FLAGS) -g $(OBJ) -fsanitize=address -o $(NAME)
	./$(NAME) $(ARG)

gitter: fclean
	git add -A
	git commit -am "Cleanup"
	git push

.PHONY: all clean fclean re runner valgrind sanitize

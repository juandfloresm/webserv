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
	git commit -am "Benchmakr with single thread connection server"
	git push

.PHONY: all clean fclean re runner valgrind sanitize


# [alert] socket: select and discovered it's not ready sock.c:384: Connection timed out
# [alert] socket: read check timed out(30) sock.c:273: Connection timed out
# [alert] socket: select and discovered it's not ready sock.c:384: Connection timed out
# [alert] socket: read check timed out(30) sock.c:273: Connection timed out

# Lifting the server siege...
# Transactions:                      0 hits
# Availability:                   0.00 %
# Elapsed time:                  59.65 secs
# Data transferred:               0.00 MB
# Response time:                  0.00 secs
# Transaction rate:               0.00 trans/sec
# Throughput:                     0.00 MB/sec
# Concurrency:                    0.00
# Successful transactions:           0
# Failed transactions:             244
# Longest transaction:            0.00
# Shortest transaction:           0.00
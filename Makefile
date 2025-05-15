NAME 				:=	webserv

CC 					:=	c++

FLAGS 				:=	-Wall -Wextra -Werror -pedantic -std=c++98

SRC 				:=	main.cpp Connection.cpp Message.cpp Request.cpp Response.cpp \
						Configuration.cpp Context.cpp Server.cpp Location.cpp

OBJ 				:=	$(SRC:.cpp=.o)

ARG					:=	./config/zweb2.conf

DOCKER_IMAGE		:= webserv
DOCKER_CONTAINER	:= webserv-dev
DOCKER_PORT			:= 8080

all: $(NAME)

$(NAME):$(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	sudo pkill webserv ; rm -f $(NAME)

re: fclean all

runner: re
	chmod -R 775 .
	sudo ./$(NAME) $(ARG)

valgrind: re
	valgrind --track-origins=yes --leak-check=full ./$(NAME) $(ARG)

fds: re
	valgrind --track-fds=yes ./$(NAME) $(ARG)

sanitize: fclean $(OBJ)
	$(CC) $(FLAGS) -g $(OBJ) -fsanitize=address -o $(NAME)
	./$(NAME) $(ARG)

siege:
	siege --time=1m --concurrent=1000 http://127.0.0.1:8080/index.php

docker-build:
	@if ! docker image inspect $(DOCKER_IMAGE) > /dev/null 2>&1; then \
		echo "Building Docker image $(DOCKER_IMAGE)..."; \
		docker build -t $(DOCKER_IMAGE) .; \
	else \
		echo "Docker image $(DOCKER_IMAGE) already exists."; \
	fi

docker-run: docker-build
	docker run --rm -d -p $(DOCKER_PORT):$(DOCKER_PORT) -v $(shell pwd):/app --name $(DOCKER_CONTAINER) $(DOCKER_IMAGE)
	@echo "Container started. Access at http://localhost:$(DOCKER_PORT)"

docker-stop:
	docker stop $(DOCKER_CONTAINER) 2>/dev/null || true

docker-shell:
	docker exec -it $(DOCKER_CONTAINER) bash

docker-logs:
	docker logs -f $(DOCKER_CONTAINER)

docker-rebuild:
	docker exec $(DOCKER_CONTAINER) make

docker-restart: docker-rebuild
	docker exec $(DOCKER_CONTAINER) bash -c "pkill -f webserv || true && make runner"

docker: docker-run
	@echo "Docker development environment ready."
	@echo "- Use 'make docker-logs' to see output"
	@echo "- Use 'make docker-restart' after code changes"
	@echo "- Use 'make docker-shell' to open a shell"

docker-clean: docker-stop
	docker rmi $(DOCKER_IMAGE) 2>/dev/null || true

gitter: fclean
	git add -A
	git commit -am "Configuration: support CGI proxy to differ between static and dynamic pages"
	git push

.PHONY: all clean fclean re runner valgrind fds sanitize \
		docker docker-build docker-run docker-stop docker-shell \
		docker-logs docker-rebuild docker-restart docker-clean

NAME 				:=	webserv

CC 					:=	c++

FLAGS 				:=	-Wall -Wextra -Werror -pedantic -std=c++98

SRC 				:=	main.cpp Connection.cpp Message.cpp Request.cpp Response.cpp \
						Configuration.cpp Context.cpp Server.cpp Location.cpp

OBJ 				:=	$(SRC:.cpp=.o)

ARG					:=	./config/zweb2.conf
WWW					:=	$(shell if [ -n "$$DOCKER_ENV" ]; then echo "/app/www"; else echo "/home/$$(whoami)/www"; fi)

TESTS_BASE			:=	./config/_tests

DOCKER_FILE			:=	./config/_tests/Dockerfile
DOCKER_IMAGE		:=	webserv
DOCKER_CONTAINER	:=	webserv-dev
DOCKER_PORT			:=	8080

all: $(NAME)

$(NAME):$(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	pkill webserv ; rm -f $(NAME)

re: fclean all
	make clean
	mkdir -p $(WWW)/html
	mkdir -p $(WWW)/cgi-bin
	cp -ru ./config/html $(WWW)/
	cp -ru ./config/cgi-bin $(WWW)/
	chmod -R 777 $(WWW)
	chmod -R 777 ./config/bodies

runner: re
	clear
	WPATH=$(WWW) ./$(NAME) $(ARG)

runner-t: re
	@echo "Starting server in background..."
	@make clean
	@chmod -R 775 .
	@WPATH=$(WWW) ./$(NAME) $(ARG) > server.log 2>&1 & echo $$! > server.pid
	@echo "Server started with PID: $$(cat server.pid)"
	@sleep 2
	@echo "Building and running tests..."
	@if $(MAKE) -C $(TESTS_BASE)/tests/ && $(TESTS_BASE)/tests/test_basic > tests.log; then \
		echo "✅ Tests completed successfully"; \
	else \
		echo "❌ Tests failed"; \
	fi
	@echo "Shutting down server..."
	@kill $$(cat server.pid) || kill -9 $$(cat server.pid) || true
	@rm -f server.pid
	@rm -f $(TESTS_BASE)/tests/test_basic
	@echo "Test run complete."

valgrind: re
	clear
	WPATH=$(WWW) valgrind --track-origins=yes --leak-check=full ./$(NAME) $(ARG)

fds: re
	clear
	WPATH=$(WWW) valgrind --track-fds=yes ./$(NAME) $(ARG)

sanitize: fclean $(OBJ)
	$(CC) $(FLAGS) -g $(OBJ) -fsanitize=address -o $(NAME)
	WPATH=$(WWW) ./$(NAME) $(ARG)

siege:
	siege --time=1m --concurrent=1000 -b http://127.0.0.1:8080/index.php

docker-build:
	@if ! docker image inspect $(DOCKER_IMAGE) > /dev/null 2>&1; then \
		echo "Building Docker image $(DOCKER_IMAGE)..."; \
		docker build -f $(DOCKER_FILE) -t $(DOCKER_IMAGE) .; \
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

ab-check:
	@if ! command -v ab > /dev/null; then \
		sudo apt-get update && sudo apt-get install -y apache2-utils; \
	fi

wrk-check:
	@if ! command -v wrk > /dev/null; then \
		sudo apt-get update && sudo apt-get install -y wrk; \
	fi

wrk: wrk-check
	wrk -t4 -c400 -d30s http://127.0.0.1:8080/

ab: ab-check
	ab -n 10000 -c 100 http://127.0.0.1:8080/

gitter: fclean
	git add -A
	git commit -am '$(m)'
	git push

.PHONY: all clean fclean re runner valgrind fds sanitize \
		docker docker-build docker-run docker-stop docker-shell \
		docker-logs docker-rebuild docker-restart docker-clean \
		ab-check wrk-check ab wrk

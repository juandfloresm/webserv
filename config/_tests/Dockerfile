FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /opt

# Install system tools and dependencies
RUN apt-get update && apt-get install -y \
	wget curl gnupg software-properties-common \
	lsb-release ca-certificates \
	libxml2 libzip4 libsodium23 libargon2-1 \
	libssl3 libcurl4 libjpeg-turbo8 libpng16-16 \
	build-essential make git perl && \
	rm -rf /var/lib/apt/lists/*

# Install PHP 8.1 // The exact version of the campus is 8.1.2 but I had problems installing it
RUN apt-get update && \
	apt-get install -y software-properties-common && \
	add-apt-repository ppa:ondrej/php -y && \
	apt-get update && \
	apt-get install -y php8.1 php8.1-cgi && \
	php -v && \
	rm -rf /var/lib/apt/lists/*

# Install Perl 5.34.0
RUN apt-get update && \
	apt-get install -y perl && \
	perl --version | grep -q "v5.34.0" && \
	rm -rf /var/lib/apt/lists/*

# Install Python 3.10.12 from source with required dependencies
RUN apt-get update && apt-get install -y \
	build-essential libssl-dev zlib1g-dev libbz2-dev \
	libreadline-dev libsqlite3-dev libncursesw5-dev \
	xz-utils tk-dev libxml2-dev libxmlsec1-dev libffi-dev liblzma-dev && \
	wget https://www.python.org/ftp/python/3.10.12/Python-3.10.12.tgz && \
	tar -xzf Python-3.10.12.tgz && \
	cd Python-3.10.12 && \
	./configure --enable-optimizations && \
	make -j$(nproc) && \
	make altinstall && \
	cd .. && rm -rf Python-3.10.12* && \
	rm -rf /var/lib/apt/lists/*

# Install Node.js 20.15.1
RUN curl -fsSL https://nodejs.org/dist/v20.15.1/node-v20.15.1-linux-x64.tar.xz | tar -xJ && \
	mv node-v20.15.1-linux-x64 /opt/node && \
	ln -s /opt/node/bin/node /usr/bin/node && \
	ln -s /opt/node/bin/npm /usr/bin/npm

# Install g++ 10.5.0
RUN add-apt-repository ppa:ubuntu-toolchain-r/test -y && \
	apt-get update && apt-get install -y g++-10 gcc-10 && \
	update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 100 && \
	update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100 && \
	rm -rf /var/lib/apt/lists/*

# Add project files
WORKDIR /app
COPY . .

RUN chmod +x ./config/cgi-bin/*.sh ./config/cgi-bin/*.py ./config/cgi-bin/*.pl

EXPOSE 8080

ENTRYPOINT ["/bin/bash", "-c", "cd /app && make runner & tail -f /dev/null"]
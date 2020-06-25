FROM ubuntu:18.04
RUN 	apt-get -y update &&\
	apt-get -y install \
	htop \
	ninja-build\
	libprotobuf-c-dev protobuf-c-compiler\
	libi2c-dev\
	zsh \
	python3 \
	git \
	wget \
	bc \
	g++-multilib \
	gcc-8-multilib \
	libssl-dev \
	binutils-dev \
	curl \
	python \
	tmux \
	gpg \
	software-properties-common \
	vim &&\
	chsh -s /bin/zsh root &&\
	wget https://github.com/robbyrussell/oh-my-zsh/raw/master/tools/install.sh -O - | zsh || true

RUN	wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null &&\
	apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'&&\
	apt update -y &&\
	apt install -y cmake

RUN 	apt-get -y install \
	ninja-build \
	libprotobuf-c-dev protobuf-c-compiler \
	libi2c-dev
RUN chmod a+rwx -R /usr


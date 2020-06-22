# specify base image
FROM jupyter/tensorflow-notebook:dd2087c75645

# Don't change dir or copy files. Instead use docker run -v to link files
#WORKDIR ~/power_mgmt_infra/src/sorter/docker/
WORKDIR /tmp
#COPY . .

# install dependencies
RUN pip3 install keras\

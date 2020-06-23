# Docker Documentation

## Setup
- `sudo usermod -aG docker jonathan` allow user to run docker commands (without sudo)

## Dockerfile
```
# Start container
JUPY_CONTAINER_NAME="jupytest"
IMAGE="jupyter/base-notebook:latest"

HOST_WORK=`pwd`
CONTAINER_WORK=/home/jovyan/work

docker run -it --rm \
  -p 8888:8888 \
  -v "${HOST_WORK}":"${CONTAINER_WORK}" \
  $IMAGE start-notebook.sh
```

## Build & run container
- `docker build -t jonathan/test .` build and name docker image with tag `'jonathan/test'`  

- Run the container using either option
  - `docker run -it --rm -p 8888:8888 -v "$PWD":/home/jovyan/work jonathan/test`
    - Runs in interactive mode, removes container after exit
  - `docker run -d --rm -p 8888:8888 -v "$PWD":/home/jovyan/work jonathan/test`
    - Runs in background with daemon, use these commands to start and stop
      - `docker start container_name`
      - `docker stop container_name`

### Command options
  `docker run [options]` ([docs](https://docs.docker.com/engine/reference/commandline/run/))
  - `--rm` removes container after completion
  - `--name container_name` names container
  - `-d` runs container as daemon (doesn't block terminal)
    - use `docker start container_name` and `docker stop container_name`
  - `-it` runs container in interactive mode (use to get notebook URL)
  - `-v "$PWD":/home/jovyan/work` mounts current director in container to allow read/write. `jovyan` is the default user for jupyter notebooks

## Maintenance
- `docker image ls` list images  
- `docker ps -a` list all containers
- `docker stop $(docker ps -a -q)` stop all containers
- `docker rm $(docker ps -a -q)` remove all containers
- `docker rmi $(docker images -f dangling=true -q)` remove docker images with no tag

## Issues
- [Permission denied from Jupyter Notebook](https://github.com/jupyter/docker-stacks/issues/114)  
Test this in terminal.
  ```
  mkdir test
  ls -l test
  chgrp 100 test
  chmod g+w test
  ls -l test
  ```

## Resources
- [Jupyter Docker Stacks](https://jupyter-docker-stacks.readthedocs.io/en/latest/index.html)
- [Run Docker in WSL](https://nickjanetakis.com/blog/setting-up-docker-for-windows-and-wsl-to-work-flawlessly)

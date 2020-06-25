# Docker Documentation
Why use docker?  
https://nickjanetakis.com/blog/setting-up-a-python-development-environment-with-and-without-docker
https://blogs.sap.com/2018/10/28/jupyter-python-notebooks-on-docker/


## Setup
Allow user to run docker commands (without sudo)
```
sudo usermod -aG docker jonathan
```
## Dockerfile
https://medium.com/bb-tutorials-and-thoughts/docker-a-beginners-guide-to-dockerfile-with-a-sample-project-6c1ac1f17490
Maybe put run in Dockerfile like this?
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
Might try Docker-compose multiple containers (ie. train and convert_to_Darknet)
Run `docker-compose up`

## Build & run container
```
docker build -t jonathan/test /media/Data/sync/gw/research/power_mgmt_infra/src/sorter/keras/
docker run -it --rm -p 8888:8888 -v /media/Data/sync/gw/research:/home/jovyan/work jonathan/test
```

  `docker run [options]` ([docs](https://docs.docker.com/engine/reference/commandline/run/))
  - `--rm` removes container after completion
  - `--name container_name` names container
  - `-d` runs container as daemon (doesn't block terminal)
    - use `docker start container_name` and `docker stop container_name`
  - `-it` runs container in interactive mode (use to get notebook URL)
  - `-v "$PWD":/home/jovyan/work` mounts current director in container to allow read/write. `jovyan` is the default user for jupyter notebooks

Maybe run `-td`? Run as daemon, but need URL with token.

### Other commands
List images and containers.
```
docker image ls
docker ps -a
```

Stop and remove all containers.
```
docker stop $(docker ps -a -q)
docker rm $(docker ps -a -q)
```

Remove image, image with no tag, force remove all.
```
docker rmi image_name
docker rmi $(docker images -f dangling=true -q)
docker rmi $(docker images -q) --force
```

## Issues
- [Permission denied from Jupyter Notebook](https://github.com/jupyter/docker-stacks/issues/114)  
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

# allow user jonathan to use docker without super user
sudo usermod -aG docker jonathan
# builds and names docker image with tag 'jonathan/test'
docker build -t jonathan/test . 
# spawns container with image
docker run -p 8888:8888 jonathan/test
# remove docker image with no name
docker rmi $(docker images -f dangling=true -q)
# list images
docker image ls
# list running containers
docker ps
# stop all containers
docker stop $(docker ps -a -q)
# remove all containers
docker rm $(docker ps -a -q)

# run container
# --rm removes container when done
# -- name container_name (names container)
# -d lets container run in background (use docker start container_name and docker stop container_name) 
# -td runs container as a daemon
# -it runs container in interactive mode (use to get url)

# DOESN'T WORK
docker run -it -p 8888:8888 -v /home/jonathan/power_mgmt_infra/src/sorter/docker/volume:/tmp --rm --name jupyter jonathan/test
docker run -it -p 8888:8888 -v /home/jonathan/power_mgmt_infra/src/sorter/docker/volume:/tmp --rm jonathan/test
docker run -p 8888:8888 -d --name train jonathan/test
docker run -p 8888:8888 -v `pwd`:/notebooks -it jonathan/test


run in interactive mode:
docker run -it --rm -p 8888:8888 -v /home/jonathan/test:/home/jovyan/work jonathan/test

run in background with daemon:
docker run -d --rm -p 8888:8888 -v "$PWD":/home/jovyan/work jonathan/test
docker run -it -p 8888:8888 -v /home/jonathan/power_mgmt_infra/src/sorter/docker:/tmp --rm jonathan/test7
 

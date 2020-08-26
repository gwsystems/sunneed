#!/bin/sh
LIST_OF_DEPENDENCIES="libprotobuf-c1"

apt-get update
apt-get install -y $LIST_OF_DEPENDENCIES

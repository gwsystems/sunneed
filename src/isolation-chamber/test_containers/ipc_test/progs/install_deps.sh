#!/bin/sh
LIST_OF_DEPENDENCIES=""

apt-get update
apt-get install -y libprotobuf-c1 $LIST_OF_DEPENDENCIES

#!/bin/sh

set -e

docker build -t docker-hosted.taujhe.de/build/libmumble_client build-container

docker push docker-hosted.taujhe.de/build/libmumble_client

docker rmi docker-hosted.taujhe.de/build/libmumble_client
FROM ubuntu:latest

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Berlin

RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    libc++-dev \
    lld \
    cmake \
    ninja-build \
    libmsgsl-dev \
    libboost-dev \
    libboost-log-dev \
    libboost-program-options-dev \
    libboost-test-dev \
    libssl-dev \
    libprotobuf-dev \
    protobuf-compiler \
    libsoundio-dev \
    libopus-dev \
  && rm -rf /var/lib/apt/lists/*

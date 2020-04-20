# Copyright 2020 Charles Daniels
#
# This file is part of OpenVCD and is released under a BSD 3-clause license.
# See the LICENSE file in the project root for more information

# Based on: https://github.com/charlesdaniels/ikos-docker

FROM ubuntu:18.04

# as documented here: https://github.com/NASA-SW-VnV/ikos/blob/master/doc/install/UBUNTU_18.04.md

RUN apt-get update --yes && apt-get upgrade --yes && apt-get install --yes wget gnupg-utils git
RUN echo "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-7 main" | tee -a /etc/apt/sources.list && wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -

RUN apt-get update
RUN apt-get install --yes gcc g++ cmake libgmp-dev libboost-dev libboost-filesystem-dev libboost-test-dev python python-pygments libsqlite3-dev libz-dev libedit-dev llvm-7 llvm-7-dev clang-7

RUN cd /tmp && \
	git clone --branch v2.1 https://github.com/NASA-SW-VnV/ikos.git && \
	cd ikos && mkdir build && cd build && \
	cmake -DCMAKE_INSTALL_PREFIX=/usr/local/ -DLLVM_CONFIG_EXECUTABLE="/usr/lib/llvm-7/bin/llvm-config" .. && \
	make && make check && make install && rm -rf /tmp/ikos

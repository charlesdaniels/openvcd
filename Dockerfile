# Copyright 2020 Charles Daniels
#
# This file is part of OpenVCD and is released under a BSD 3-clause license.
# See the LICENSE file in the project root for more information

# Based on: https://github.com/charlesdaniels/ikos-docker
#
# Also see: https://github.com/NASA-SW-VnV/ikos/blob/master/doc/install/UBUNTU_18.04.md

FROM ubuntu:18.04

#### ikos installation ########################################################

RUN DEBIAN_FRONTEND=noninteractive apt-get update -yq && apt-get upgrade --yes && apt-get install --yes wget gnupg-utils git
RUN echo "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-9 main" | tee -a /etc/apt/sources.list && wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -

RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -yq gcc g++ cmake libgmp-dev libboost-dev libboost-filesystem-dev \
    libboost-thread-dev libboost-test-dev python python-pygments libsqlite3-dev libtbb-dev \
    libz-dev libedit-dev llvm-9 llvm-9-dev llvm-9-tools clang-9


RUN cd /tmp && \
	git clone --branch v3.0 https://github.com/NASA-SW-VnV/ikos.git && \
	cd ikos && mkdir build && cd build && \
	cmake -DCMAKE_INSTALL_PREFIX=/usr/local/ -DLLVM_CONFIG_EXECUTABLE="/usr/lib/llvm-9/bin/llvm-config" .. && \
	make && make check && make install && rm -rf /tmp/ikos

#### lizard installation ######################################################

RUN DEBIAN_FRONTEND=noninteractive apt-get install -yq python3 python3-pip
RUN pip3 install  lizard

#### valgrind #################################################################

RUN DEBIAN_FRONTEND=noninteractive apt-get install -yq valgrind

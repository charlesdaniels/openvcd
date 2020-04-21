#!/bin/sh

# Copyright 2020 Charles Daniels
#
# This file is part of OpenVCD and is released under a BSD 3-clause license.
# See the LICENSE file in the project root for more information

# Executes the OpenVCD test suite under docker. You must have docker available
# to run this.
#
# Use -i if you want to interact with it afterwards.

set -x

cd "$(dirname "$0")"

docker build -t openvcd_test_image .

if [ "$1" = "-i" ] ; then
	docker run -e OPENVCD_TEST_INTERACTIVE=YES -it -p 8080:8080 -v "$(pwd)":/mnt openvcd_test_image sh /mnt/testscript.sh 
else
	docker run -v "$(pwd)":/mnt openvcd_test_image sh /mnt/testscript.sh
fi

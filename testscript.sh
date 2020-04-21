#!/bin/sh

# Copyright 2020 Charles Daniels
#
# This file is part of OpenVCD and is released under a BSD 3-clause license.
# See the LICENSE file in the project root for more information

# You should not run this manually, it is used by ./runtests.sh to run the
# tests inside of the Docker container.

set -x

# this should be /mnt
cd "$(dirname "$0")"

if [ "$(pwd)" != "/mnt" ] ; then
	echo "pwd ($(pwd)) is not /mnt, are you running this manually? don't do that." 1>&2
	exit 1
fi

mkdir -p /tmp/src
cp -r /mnt/ /tmp/src/
cd /tmp/src/mnt/

FAILED=NO

make -C src clean
if ! make RUN_TESTS=YES TEST_WITH_VALGRIND=YES CFLAGS="-std=c99 -Wall -Wextra -pedantic -Werror -O0 -g3" -C src ; then
	FAILED=YES
fi

make -C src clean
yes | ikos-scan make -C src/ > ikos.log 2>&1

# if grep "The program is definitely UNSAFE" < ./ikos.log ; then
#         FAILED=YES
# fi
# 
# if grep "The program is potentially UNSAFE" < ./ikos.log ; then
#         FAILED=YES
# fi

if [ "$OPENVCD_TEST_INTERACTIVE" = "YES" ] ; then
	bash
fi

if [ "$FAILED" = "NO" ] ; then
	exit 0
else
	exit 1
fi

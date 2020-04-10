# Copyright 2020 Charles Daniels
#
# This file is part of OpenVCD and is released under a BSD 3-clause license.
# See the LICENSE file in the project root for more information

# Usage: sh ./test_with_valgrind.sh ./something.test
#
# Runs an OpenVCD test under valgrind, and exits with an error if any blocks
# are reported as possibly lost.

VALGRIND_OUTPUT="$(valgrind $1 2>&1)"

if [ $(echo "$VALGRIND_OUTPUT" | awk '($2 =="definitely" && $3 == "lost:" && $4 > 0)' | wc -l) -gt 0 ] ; then
	echo "$VALGRIND_OUTPUT" 1>&2
	echo "MEMORY LEAK DETECTED!" 1>&2
	exit 1
fi

if [ $(echo "$VALGRIND_OUTPUT" | awk '($2 =="ERROR" && $3 == "SUMMARY:" && $4 > 0)' | wc -l) -gt 0 ] ; then
	echo "$VALGRIND_OUTPUT" 1>&2
	echo "MEMORY ERRORS DETECTED!" 1>&2
	exit 1
fi

# get the exit code if the test fails normally
$1

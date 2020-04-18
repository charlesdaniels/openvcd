# Copyright 2020 Charles Daniels
#
# This file is part of OpenVCD and is released under a BSD 3-clause license.
# See the LICENSE file in the project root for more information

include ../opinionated.mk
include ../config.mk

OBJ = parser.o util.o

ifeq "$(TEST_WITH_VALGRIND)" "YES"
	TESTCMD = sh ./test_with_valgrind.sh
else
	TESTCMD =
endif

tests: parser.test util.test
> if [ "$(RUN_TESTS)" = "YES" ] ; then $(TESTCMD) ./parser.test ; fi
> if [ "$(RUN_TESTS)" = "YES" ] ; then $(TESTCMD) ./util.test ; fi
.PHONY: tests

%.test: $(OBJ) %.test.c
> $(CC) $(CFLAGS) -o $@ $^

%.o: %.c %.h
> $(CC) $(CFLAGS) -c $<

%.o: %.c
> $(CC) $(CFLAGS) -c $<

clean:
> rm -f *.o *.test
.PHONY: clean

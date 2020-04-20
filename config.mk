CC ?= gcc
CFLAGS ?= -std=c99 -Wall -Wextra -pedantic -O0 -g3

# set to 'YES' use valgrind to search for memory errors while testing
TEST_WITH_VALGRIND ?= YES

# set to 'YES' to run tests as part of the build process
RUN_TESTS ?= YES

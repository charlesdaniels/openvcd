/* Copyright 2020 Charles Daniels
 *
 * This file is part of OpenVCD and is released under a BSD 3-clause license.
 * See the LICENSE file in the project root for more information */

#define _GNU_SOURCE
#include "stdio.h"

#include "test_util.h"
#include "util.h"

void test_charfilter(void) {
	char* teststr1 = "abcd ;,!$ 12345 xyz";

	char* res = openvcd_charfilter(teststr1, "abcdxyz");
	str_should_equal(res, "abcdxyz");
	free(res);
	res = openvcd_charfilter(teststr1, "54321xaybcdz");
	str_should_equal(res, "abcd12345xyz");
	free(res);

}

int main(void) {
	test_charfilter();

	return 0;
}

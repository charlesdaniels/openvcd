#include "util.h"


char* openvcd_charfilter(char* s, char* filter) {
	char* r;
	size_t rp;

	r = malloc(sizeof(char) * (strlen(s) + 1));	
	r[0] = '\0';
	rp = 0;
	
	for (size_t i = 0 ; i < strlen(s) ; i++) {
		for (size_t j = 0 ; j < strlen(filter) ; j++) {
			if (s[i] == filter[j]) {
				goto i_in_filter;
			}
		}

		continue;

i_in_filter:
		r[rp] = s[i];
		rp ++;
		r[rp] = '\0';
	}

	return r;
}

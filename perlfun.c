/*
 *  Copyright (C) 2009 Michael Fung <mike@3open.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string.h>
#include <ctype.h>
#include "perlfun.h"


char *trim(char *buf) {

	/* NOTE: the buf will be modified, make backup before calling this
	 function if needed */
	int i;
	char *result;

	result = buf;
	if (strlen(result) == 0) {
		return result;
	}

	// left trim
	for (i=0; i < strlen(result); i++) {

		if (isspace(result[0])) {
			result++;
		} else {
			break;
		}
	}

	// right trim
	for (i = strlen(result)-1; i >= 0; i--) {
		if (!isspace(result[i])) break; // break at the real char
	}
	result[i+1] = '\0';  // mark ending , use i+1 as we are in the last real char

 return result;
} // end trim

int split(char *buf, char *tokens[], char *delimiters) {

    char *token;
    int ntokens = 0;

    token = strtok(buf, delimiters);
    while (token != NULL) {
        tokens[ntokens] = token;  // JUST COPY THE POINTER
        ntokens++;
        token = strtok(NULL, delimiters);
        }

        return ntokens;
}

void chomp(char *line) {

    /* NOTE: the line[] will be modified, make backup before calling this
     function if needed */
	int i;

	// chop off \r and \n
	for (i = (strlen(line) - 1); i >= 0; i--) {

		if (line[i] == '\n' || line[i] == '\r') {
            line[i] = '\0';  // replace ending of string
		} else {
			break;
		}
	}
}


int strindex(const char *haystack, const char *needle) {
    int result;
    char *p;

    if ((p = strstr(haystack, needle)) != NULL) {
        result = p - haystack;
        return result;
    } else {
        return -1;  // return -1 if needle NOT FOUND
    }
}

void substr(const char *string, char *buf, int start, int length) {

    int i;
    int str_end = strlen(string);

    // check mad length
    if ((start + length) > str_end) length = -1;

    // length of -1 means to the end
    if (length == -1) length = str_end - start;

    for (i = 0; i < length; i++) {
        buf[i] = string[i+start];
    }
    buf[i] = '\0';
}

/*
int packhex(const char *hexstr, unsigned char *bytes, int num_bytes) {

    int i,j=0;
    char upper;

    // error checking
    if ((strlen(hexstr) / 2) < num_bytes) return -1;  // error!

    for (i = 0; i < (num_bytes * 2); i++) {

        // high nibble
        upper = toupper(hexstr[i]);
        if ((upper >= 'A') && (upper <= 'F')) {  // A-F
            bytes[j] = ((unsigned char)upper - 55) * 16;
        } else {
            if ((upper >= '0') && (upper <= '9')) { // 0-9
                bytes[j] = ((unsigned char)upper - 48) * 16;
            } else {
                bytes[j] = 0;
            }
        }

        // low nibble
        upper = toupper(hexstr[++i]);
        if ((upper >= 'A') && (upper <= 'F')) {  // A-F
            bytes[j] = bytes[j] + ((unsigned char)upper - 55);
        } else {
            if ((upper >= '0') && (upper <= '9')) { // 0-9
                bytes[j] = bytes[j] + ((unsigned char)upper - 48);
            } else {
                bytes[j] = 0;
            }
        }
        j++;  // next byte
    }

    return j;  // return num of bytes converted
}
*/

char *shift(char *buf) {
	char *result;
	result = buf;
	result++;
	return result;
}


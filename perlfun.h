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

/* pack hex string
	Usage example:
	unsigned char bytes[4];
	char hexstr[256];
	fgets(hexstr, 256, stdin);
	if (packhex(hexstr, bytes, sizeof(bytes)) > 0) {
		for (i=0; i<4; i++) {
			printf("%x:%i\n", bytes[i], bytes[i]);
		}
	}
*/

/*
	extern int packhex(const char *hexstr, unsigned char *bytes, int num_bytes);
*/


/*
	copy a substring of string, starting at start with specific length,
	into the buffer pointed to by buf.
	note: length of -1 means to the end
*/
extern void substr(const char *string, char *buf, int start, int length);


/*
	return the position of needle in the haystack, -1 if not found
*/
extern int strindex(const char *haystack, const char *needle);


/*
	chop off \n and \r at the end of line, buffer pointed to by line will
	be modified.
*/
extern void chomp(char *line);


extern int split(char *buf, char *tokens[], char *delimiters);


extern char *trim(char *buf);


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

#include "miniupdate.h"


int getifaddr(const char *ifname, char *ipaddr) {

	/* init vars */
	int numreqs = 30, sd, n, tick, found=0;
	struct ifconf ifc;
	struct ifreq *ifr;
	struct in_addr *ia;
	char *tmpip;

	/* cheap way to empty the result */
	*ipaddr = '\0';

	sd=socket(AF_INET, SOCK_STREAM, 0);
	ifc.ifc_buf = NULL;
	ifc.ifc_len = sizeof(struct ifreq) * numreqs;

	if ((ifc.ifc_buf = realloc(ifc.ifc_buf, ifc.ifc_len)) == NULL) {
		logwrite("getifaddr: Fatal error in memory allocation");
		close(sd);
		prg_exit(EXIT_FAILURE);
		return(-1);
	}

	if (ioctl(sd, SIOCGIFCONF, &ifc) < 0) {
		free(ifc.ifc_buf);
		close(sd);
		logwrite("getifaddr: error in ioctl call");
		return(-1);
	}

	ifr = ifc.ifc_req;
	for (n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq)) {

		ia= (struct in_addr *) ((ifr->ifr_ifru.ifru_addr.sa_data)+2);

		if ((tick = strcmp(ifr->ifr_ifrn.ifrn_name, ifname)) == 0) {
			tmpip = inet_ntoa(*ia);
			if (log_verbose) {
				sprintf(msg, "getifaddr: IP of %s:%s", ifname, tmpip);
				logwrite(msg);
			}
			if (sscanf(tmpip, "%u.%u.%u.%u", &int_tmp,&int_tmp,&int_tmp,&int_tmp) == 4) {
				strncpy(ipaddr, tmpip, IPADDR_LEN);
				found = 1;
			} else {
				if (log_verbose) logwrite("getifaddr: error in IP format");
			}
			break;
		}

		ifr++;
	}

	free(ifc.ifc_buf);
	close(sd);

	if (found) {
		return(0);
	} else {
		return(-1);
	}

}


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


int parseconf(char *conf_file)
{
	/* local dynamic variables */
	FILE *conf_fd;
	char buf[MAX_CONF_LINE];
	char *pbuf;
	char key[MAX_CONF_LINE], value[MAX_CONF_LINE], *pkey, *pvalue;
	int offset;

	/* local static variables */
	//static unsigned char parsed = 0;

	/* init the configuration to defaults */
	login_id[0] = 0;
	hostname[0] = 0;
	password[0] = 0;
	md5password[0] = 0;
	ifname[0] = 0;
	on_change[0] = 0;
	strcpy(myip_server, DEFAULT_MYIP_SERVER);
	myip_server_port = DEFAULT_MYIP_SERVER_PORT;
	strcpy(nsupdate_server, DEFAULT_NSUPDATE_SERVER);
	nsupdate_server_port = DEFAULT_NSUPDATE_SERVER_PORT;
	no_public_ip = DEFAULT_NO_PUBLIC_IP;
	log_verbose = DEFAULT_LOG_VERBOSE;
	use_syslog = DEFAULT_USE_SYSLOG;
	no_logging = DEFAULT_NO_LOGGING;

	/* change of pid_file, log_file requires program restart */
	/* only set pid and log file if not yet parsed */
	//if(parsed == 0) {
		//strncpy(pid_file, DEFAULT_PID_FILE, MAX_FILE_PATH);
		pid_file = DEFAULT_PID_FILE;
		strncpy(log_file, DEFAULT_LOG_FILE, MAX_FILE_PATH);
	//}

	/* open the conf file */
	if ((conf_fd = fopen(conf_file, "r")) == NULL) {
		printf("parseconf: Error opening config file for reading\n");
		return -1;
	}

	while(fgets(buf, MAX_CONF_LINE, conf_fd) != NULL) {
		//strncpy(line, buf, sizeof(line));
		pbuf = trim(buf);
		if ((pbuf[0] == '#') || (strlen(pbuf) == 0 )) continue;  //skip comments or blank lines
		//printf("%s\n", pbuf);
		offset = strindex(pbuf, "=");
		if ((offset < 1) || (offset == (strlen(pbuf) - 1))) continue;  // skip invalid lines

		substr(pbuf, key, 0, offset);
		substr(pbuf, value, offset + 1, -1);

		//printf("raw key/value:(%s)=(%s)\n", key, value);
		pkey = trim(key);
		pvalue = trim(value);
		//printf("Trimed:(%s) = (%s)\n", pkey, pvalue);

		/* start parsing the key/value pair */
		if ((strcmp(pkey,"login_id") == 0) && (strlen(pvalue) > 0)) {
			strncpy(login_id, pvalue, sizeof(login_id));
			continue;
		}
		else if ((strcmp(pkey,"password") == 0) && (strlen(pvalue) > 0)) {
			strncpy(password, pvalue, sizeof(password));
			continue;
		}
		else if ((strcmp(pkey,"md5password") == 0) && (strlen(pvalue) > 0)) {
			strncpy(md5password, pvalue, sizeof(md5password));
			continue;
		}
		else if ((strcmp(pkey,"hostname") == 0) && (strlen(pvalue) > 0)) {
			strncpy(hostname, pvalue, sizeof(hostname));
			continue;
		}
		else if ((strcmp(pkey,"interface") == 0) && (strlen(pvalue) > 0)) {
			strncpy(ifname, pvalue, sizeof(ifname));
			continue;
		}
		else if ((strcmp(pkey,"no_public_ip") == 0) && (strlen(pvalue) > 0)) {
			if ((strcmp(pvalue,"yes") == 0) || (strcmp(pvalue,"true") == 0)) {
				no_public_ip = 1;
			} else {
				no_public_ip = 0;
			}
			continue;
		}
		else if ((strcmp(pkey,"myip_server_addr") == 0) && (strlen(pvalue) > 0)) {
			strncpy(myip_server, pvalue, sizeof(myip_server));
			continue;
		}
		else if ((strcmp(pkey,"myip_server_port") == 0) && (strlen(pvalue) > 0)) {
			if ((myip_server_port = atoi(pvalue)) > 0) {
				continue;
			} else {
				printf("parseconf: Invalid option value for myip_server_port\n");
				return -1;
			}
		}
		else if ((strcmp(pkey,"nsupdate_server_addr") == 0) && (strlen(pvalue) > 0)) {
			strncpy(nsupdate_server, pvalue, sizeof(nsupdate_server));
			continue;
		}
		else if ((strcmp(pkey,"nsupdate_server_port") == 0) && (strlen(pvalue) > 0)) {
			if ((nsupdate_server_port = atoi(pvalue)) > 0) {
				continue;
			} else {
				printf("parseconf: Invalid option value for nsupdate_server_port\n");
				return -1;
			}
		}

		//else if ((strcmp(pkey,"pid_file") == 0) && (strlen(pvalue) > 0)/* && (!parsed)*/) {
		//	strncpy(pid_file, pvalue, MAX_FILE_PATH);
		//	continue;
		//}

		else if ((strcmp(pkey,"log_file") == 0) && (strlen(pvalue) > 0)/* && (!parsed)*/) {
			strncpy(log_file, pvalue, MAX_FILE_PATH);
			continue;
		}

                else if ((strcmp(pkey,"on_change") == 0) && (strlen(pvalue) > 0)/* && (!parsed)*/) {
                        strncpy(on_change, pvalue, MAX_FILE_PATH);
                        continue;
                }


		else if ((strcmp(pkey,"log_verbose") == 0) && (strlen(pvalue) > 0)) {
			if ((strcmp(pvalue,"yes") == 0) || (strcmp(pvalue,"true") == 0)) {
				log_verbose = 1;
			} else {
				log_verbose = 0;
			}
			continue;
		}
		else if ((strcmp(pkey,"use_syslog") == 0) && (strlen(pvalue) > 0)) {
			if ((strcmp(pvalue,"yes") == 0) || (strcmp(pvalue,"true") == 0)) {
				use_syslog = 1;
			} else {
				use_syslog = 0;
			}
			continue;
		}
		else if ((strcmp(pkey,"no_logging") == 0) && (strlen(pvalue) > 0)) {
			if ((strcmp(pvalue,"yes") == 0) || (strcmp(pvalue,"true") == 0))  {
				no_logging = 1;
			} else {
				no_logging = 0;
			}
			continue;
		}
		//else if ((strcmp(pkey,"daemon_mode") == 0) && (strlen(pvalue) > 0)) {
		//	if ((strcmp(pvalue,"yes") == 0) || (strcmp(pvalue,"true") == 0)) {
		//		daemon_mode = 1;
		//	} else {
		//		daemon_mode = 0;
		//	}
		//	continue;
		//}
	}

	//parsed = 1;

	/* check required params */
	if (strlen(login_id) == 0) {
		printf("parseconf: missing <login_id>\n");
		return(-1);
	}
	else if (strlen(hostname) == 0) {
		printf("parseconf: missing <hostname>\n");
		return(-1);
	}
	else if ((strlen(password) == 0) && (strlen(md5password) == 0)) {
		printf("parseconf: missing <password> or <md5password>\n");
		return(-1);
	}
	else if ((!no_detect) && (!no_public_ip) && (strlen(ifname) == 0)) {
		printf("parseconf: missing <interface>\n");
		return(-1);
	}
	else {
		return(0);
	}
}


void dumpconf() {

	printf("\n\n*** Dumping configuration ***\n\n");
	printf("Login ID: (%s)\n", login_id);
	printf("Passowrd: (%s)\n", password);
	printf("MD5Passowrd: (%s)\n", md5password);
	printf("Hostname: (%s)\n", hostname);
	printf("myip_server: (%s)\n", myip_server);
	printf("myip_server_port: (%d)\n", myip_server_port);
	printf("nsupdate_server: (%s)\n", nsupdate_server);
	printf("nsupdate_server_port: (%d)\n", nsupdate_server_port);
	printf("log_verbose: (%d)\n", log_verbose);
	printf("use_syslog: (%d)\n", use_syslog);
	printf("no_logging: (%d)\n", no_logging);
	//printf("pid_file: (%s)\n", lock_file);
	printf("log_file: (%s)\n", log_file);
}

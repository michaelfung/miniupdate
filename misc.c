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


/* put the result into the buffer pointed to by myip */
/* return -1 and set myip to empty string if failed to get it */
int get_myip(char *myip) {

	/* define local static variables */
	static unsigned char tries = 0;
	//static unsigned char timeout = 1;
	static int myip_sockfd;
	static struct hostent *myip_server_ent;
	static struct sockaddr_in myip_servaddr;
	static char data[] = "\n";
	static char tmpip[IPADDR_LEN];
	static ssize_t n;	/* TODO: may use a global var */
	//static struct sigaction action;
	static struct timeval tv;  /* TODO: may use a global var */

	/* init vars */
	tries = 0;
	tv.tv_sec = MYIP_TIMEOUT;
	tv.tv_usec = 0;
	bzero(tmpip, IPADDR_LEN);
	// bzero(myip, IPADDR_LEN);
	*myip = '\0';
	bzero(&myip_servaddr, sizeof(myip_servaddr));
	myip_servaddr.sin_family = AF_INET;
	myip_servaddr.sin_port = htons(myip_server_port);


	/* initialise signal handler
	bzero(&action, sizeof(action));
	action.sa_handler = timed_out;
	action.sa_flags = 0;
	sigaction(SIGALRM, &action, 0); */

	/* get IP of myip_server, and setup myip_servaddr */
	myip_server_ent = gethostbyname(myip_server);
	if (myip_server_ent == NULL) {
		logwrite("get_myip: Error resolving myip_server!");
		return(-1);
	}
	memcpy(&myip_servaddr.sin_addr.s_addr, myip_server_ent->h_addr_list[0], myip_server_ent->h_length);

	myip_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	setsockopt(myip_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	/* try a few times to read myip */
	while (tries < MAX_MYIP_TRIES) {
		sendto(myip_sockfd, data, 1, 0, (SA *)&myip_servaddr, sizeof(myip_servaddr));
		//alarm(MYIP_TIMEOUT);
		if((n = recvfrom(myip_sockfd, tmpip, IPADDR_LEN, 0, NULL, NULL)) <= 0) {
			if (log_verbose) logwrite("get_myip: Timeout reading myip_server");
			++tries;
		} else {
			chomp(tmpip);
			if (log_verbose) {
				sprintf(msg, "get_myip: %s", tmpip);
				logwrite(msg);
			}
			break;  /* got answer packet */
		}
	}

	//alarm(0);
	close(myip_sockfd);

	/* make sure tmpip is in correct format */
	if (sscanf(tmpip, "%u.%u.%u.%u", &int_tmp,&int_tmp,&int_tmp,&int_tmp) != 4) {
		return(-1);
	} else {
		strncpy(myip, tmpip, IPADDR_LEN);
		return(0);
	}
}


int read_server_cooked(int sockfd, char *response) {

	char buf[MAX_RECV_LINE];
	char status_code[16];
	ssize_t n;
	int space_offset;

	if ((n = readline(sockfd, buf, MAX_RECV_LINE)) < 0) {
		return(-1);
	}

	chomp(buf);
	if (log_verbose) {
		sprintf(msg,"update server --> %s", buf);
		logwrite(msg);
	}

	space_offset = strindex(buf, " ");
	substr(buf, status_code, 0, space_offset);
	substr(buf, response, space_offset +1, -1);

	if (strcmp(status_code,"ERR") == 0) {
		/* fatal error */
		sprintf(msg,"!!FATAL ERROR!! %s", response);
		logwrite(msg);
		prg_exit(1);
	}

	return(n);
}


int ns_update(const char *ip_addr) {

	/* define local variables */
	int nsupdate_sockfd;
	struct hostent *nsupdate_server_ent;
	struct sockaddr_in nsupdate_servaddr;
	ssize_t n;
	struct timeval tv;
	char inbuf[MAX_RECV_LINE];
	char outbuf[MAX_RECV_LINE];
	char tmpbuf[65];
	char md5response[33];

	/* init vars */
	tv.tv_sec = NSUPDATE_TIMEOUT;
	tv.tv_usec = 0;
	bzero(inbuf, MAX_RECV_LINE);
	bzero(outbuf, MAX_RECV_LINE);
	bzero(&nsupdate_servaddr, sizeof(nsupdate_servaddr));
	nsupdate_servaddr.sin_family = AF_INET;
	nsupdate_servaddr.sin_port = htons(nsupdate_server_port);
	bzero(tmpbuf, 65);
	bzero(md5response, 33);


	/* get IP of nsupdate_server, and setup nsupdate_servaddr */
	nsupdate_server_ent = gethostbyname(nsupdate_server);
	if (nsupdate_server_ent == NULL) {
		logwrite("ns_update: Error resolving nsupdate_server");
		return(-1);
	}
	memcpy(&nsupdate_servaddr.sin_addr.s_addr, nsupdate_server_ent->h_addr_list[0], nsupdate_server_ent->h_length);

    if ((nsupdate_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		logwrite("ns_update: Error opening socket");
		return(-1);
	}

    /* set timeout */
	setsockopt(nsupdate_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    if (connect(nsupdate_sockfd,(SA *)&nsupdate_servaddr,sizeof(nsupdate_servaddr)) < 0) {
        logwrite("ns_update: Error connecting to nsupdate_server");
        close(nsupdate_sockfd);
        return(-1);
	}

	/* read the welcome message */
	if (read_server_cooked(nsupdate_sockfd, inbuf) < 0) {
        logwrite("ns_update: Error getting welcome message");
        close(nsupdate_sockfd);
        return(-1);
	}

	/* send agent identity */
	n = sprintf(outbuf, "agent %s/%s\n", PROG_NAME, VERSION);
	Writen(nsupdate_sockfd, outbuf, n);
	if (read_server_cooked(nsupdate_sockfd, inbuf) < 0) {
        logwrite("ns_update: Error after send agent identity");
        close(nsupdate_sockfd);
        return(-1);
	}


	/* send login command */
	n = sprintf(outbuf, "login %s digest-md5-text\n", login_id);
	Writen(nsupdate_sockfd, outbuf, n);
	if (read_server_cooked(nsupdate_sockfd, inbuf) < 0) {
        logwrite("ns_update: Error after send login command");
        close(nsupdate_sockfd);
        return(-1);
	}

	/* now the challange is in the inbuf, calculate the response */

	if (strlen(md5password) == 0) {
		MD5Str(password, tmpbuf);	/* gen md5 string of the password */
	} else {
		strncpy(tmpbuf, md5password, 32);
	}
	strncat(tmpbuf, inbuf, 32); /* glue md5 of password and challenge */
	MD5Str(tmpbuf, md5response);

	/* send response */
	n = sprintf(outbuf, "response %s\n", md5response);
	Writen(nsupdate_sockfd, outbuf, n);
	if (read_server_cooked(nsupdate_sockfd, inbuf) < 0) {
        logwrite("ns_update: Error after send response");
        close(nsupdate_sockfd);
        return(-1);
	}

	/* send update request */
	n = sprintf(outbuf, "a_update online %s %s\n", hostname, ip_addr);
	Writen(nsupdate_sockfd, outbuf, n);
	if (read_server_cooked(nsupdate_sockfd, inbuf) < 0) {
        logwrite("ns_update: Error after update request");
        close(nsupdate_sockfd);
        return(-1);
	}

	/* send exit command */
	n = sprintf(outbuf, "%s\n", "exit");
	Writen(nsupdate_sockfd, outbuf, n);
	read_server_cooked(nsupdate_sockfd, inbuf); /* read last message */

	return(0);
	close(nsupdate_sockfd);
}


void Writen(int fd, const void *vptr, size_t n) {

	writen(fd, vptr, n);

	if (log_verbose) {
		((char *)vptr)[n-1] = '\0';  /* remove the last \n char */
		sprintf(msg, "update server <-- %s", (char *)vptr);
		logwrite(msg);
	}
}


void MD5Str(char *data, char *md5sum) {

	MD5_CTX myctx;
	MD5_CTX *ctx = &myctx;
	unsigned char digest[16];
	int i;

	MD5_Init(ctx);
	MD5_Update(ctx, data, strlen(data));
	MD5_Final(digest, ctx);
	for (i = 0; i < 16; i++) {
		sprintf(md5sum, "%02x", digest[i]);
		md5sum++;
		md5sum++;
	}
}

void print_md5digest(char *data) {

	char md5sum[33];
	bzero(md5sum, 33);
	MD5Str(data, md5sum);
	printf("%s\n", md5sum);
}



void logwrite(char *s) {

	static char date_text[] = "0000-00-00 00:00:00";
	static time_t curtime;
	static struct tm *loctime;

	if (no_logging) return;

	if (!daemon_mode) {
		printf("%s\n", s);
		return;  /* non daemon mode disable logging to syslog or log file */
	}

	if (use_syslog) {
		syslog(LOG_INFO, "%s", s);

	} else if (log_fd != NULL) {
		/* Get the current time. */
		curtime = time (NULL);

		/* Convert it to local time representation. */
		loctime = localtime (&curtime);

		/* format the time to a string */
		strftime(date_text, 20, "%Y-%m-%d %H:%M:%S", loctime);

		/* write it */
		fprintf(log_fd, "[%s] %s\n", date_text, s);
	}

}


int file_exists(const char *filename) {

	FILE *fp;
	
    if (fp = fopen(filename, "r"))
    {
        fclose(fp);
        return(1);
    }
    return(0);
}


void sigterm_handler() {
	prg_exit(0);
}

void prg_exit(int exit_code) {

	if (daemon_mode) logwrite("miniupdate terminated.\n");

	//if (pid_file != NULL) free(pid_file);
	if (log_file != NULL) free(log_file);
	if (log_fd != NULL) {
		fclose(log_fd);
	}
	unlink(pid_file);
	exit(exit_code);
}

void help() {

    printf("\n");
    printf("miniDNS.net DNS Record Updater v%s\n", VERSION);
    printf("Copyright (C) 2011 Michael Fung <mike@3open.org> \n\n");

    printf("  -h              Show help\n");
	printf("  -c FILE         Read configuration from FILE\n");
	printf("  -a IPADDR       Skip IP address detection, use IPADDR instead\n");
	printf("  -d              Run in daemon mode\n");
	printf("  -q              Be quiet, only output fatal error messages\n");
	printf("  -v              Verbose output\n");
	printf("  --md5 PASSWORD  Output the MD5 digest of string PASSWORD\n");
	printf("\n");
	printf("Note: options -a, -v and --md5 do not work with daemon mode.\n");

	printf("\n");

	return;
}


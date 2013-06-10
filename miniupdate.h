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

#ifndef _MINIUPDATE_H
#define _MINIUPDATE_H


/* include system libs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/wait.h>


/* include modules we used */
#include "perlfun.h"
#include "md5.h"

/* define my types */

/* define macros */
#define SA 								struct sockaddr


/* define constants */
#define PROG_NAME		 				"miniupdate"
#define VERSION							"0.8"

#define DEFAULT_MYIP_SERVER				"myip.minidns.net"
#define DEFAULT_MYIP_SERVER_PORT 		9121
#define DEFAULT_NSUPDATE_SERVER 		"update.minidns.net"
#define DEFAULT_NSUPDATE_SERVER_PORT 	9120
#define DEFAULT_CHECK_PERIOD			60
#define DEFAULT_NET_RETRY_PERIOD		3600
#define DEFAULT_MAX_INTERVAL			86400
#define DEFAULT_CONF_FILE				"/etc/miniupdate.conf"
#define DEFAULT_PID_FILE 				"/var/run/miniupdate.pid"
#define DEFAULT_LOG_FILE				"/var/log/miniupdate.log"
#define DEFAULT_NO_PUBLIC_IP			0
#define DEFAULT_LOG_VERBOSE				0
#define DEFAULT_USE_SYSLOG				0
#define DEFAULT_NO_LOGGING				0
#define DEFAULT_DAEMON_MODE				0
#define MAX_IFNAME						16
#define MAX_FILE_PATH					256
#define MAX_CONF_LINE					256
#define MAX_RECV_LINE					256
#define MAX_HOSTNAME					128
#define MAX_LOGIN_ID					16
#define MAX_PASSWORD					16
#define IPADDR_LEN						16
#define MYIP_TIMEOUT					5
#define MAX_MYIP_TRIES					3
#define NSUPDATE_TIMEOUT				10
#define BACKOFF_INTERVAL1				300		/* 5 min */
#define BACKOFF_INTERVAL2				1800	/* 30 min */
#define BACKOFF_INTERVAL3				3600	/* 1 hr */


/* declare global variables */
extern char login_id[MAX_LOGIN_ID];
extern char hostname[MAX_HOSTNAME];
extern char password[MAX_PASSWORD];
extern char md5password[32];
extern char ifname[MAX_IFNAME];
extern char myip_server[MAX_HOSTNAME];
extern int myip_server_port;
extern char nsupdate_server[MAX_HOSTNAME];
extern int nsupdate_server_port;
extern char *conf_file;
extern char *pid_file;
extern char *log_file;
extern FILE *pid_fd;
extern FILE *log_fd;
extern unsigned char no_public_ip;
extern unsigned char log_verbose;
extern unsigned char use_syslog;
extern unsigned char no_logging;
extern unsigned char daemon_mode;
extern unsigned char quiet_mode;
extern unsigned char verbose_mode;
extern unsigned char no_detect;
extern int int_tmp;
extern char last_ip[IPADDR_LEN];
extern char current_ip[IPADDR_LEN];
extern char msg[512];  /* for log msg generation use */
extern char on_change[MAX_FILE_PATH];

/* define prototypes */
extern int parseconf(char *conf_file);
extern void dumpconf();
extern void logwrite(char *msg);
extern int get_myip(char *myip);
extern int read_server_cooked(int sockfd, char *response);
extern int ns_update(const char *ip_addr);
extern void sigterm_handler();
extern void prg_exit(int exit_code);
extern ssize_t readline(int fd, void *vptr, size_t maxlen);
extern ssize_t writen(int fd, const void *vptr, size_t n);
extern void Writen(int fd, const void *vptr, size_t n);
extern void MD5Str(char *data, char *md5sum);
extern void print_md5digest(char *data);
extern void help();
extern int getifaddr(const char *ifname, char *ipaddr);
extern int file_exists(const char *filename);

#endif

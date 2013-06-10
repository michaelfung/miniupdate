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

/* define global variables */
char login_id[MAX_LOGIN_ID];
char hostname[MAX_HOSTNAME];
char password[MAX_PASSWORD];
char md5password[32];
char ifname[MAX_IFNAME];
char myip_server[MAX_HOSTNAME];
int myip_server_port;
char nsupdate_server[MAX_HOSTNAME];
int nsupdate_server_port;
char *conf_file;
char *pid_file;
char *log_file;
FILE *pid_fd;
FILE *log_fd;
unsigned char no_public_ip;
unsigned char log_verbose;
unsigned char use_syslog;
unsigned char no_logging;
unsigned char daemon_mode;
unsigned char quiet_mode;
unsigned char verbose_mode;
unsigned char no_detect;
int int_tmp;
char last_ip[IPADDR_LEN];
char current_ip[IPADDR_LEN];
char msg[512];
char on_change[MAX_FILE_PATH];

/* MAIN ENTRANCE */
int main(int argc, char *argv[]) {

	/* define local dynamic variables */
	int i, ret;
	struct sigaction action;
	pid_t pid;
	unsigned int update_error_count, check_period;


	/* init vars */
	pid_fd = NULL;
	log_fd = NULL;
	pid_file = DEFAULT_PID_FILE;
	daemon_mode = DEFAULT_DAEMON_MODE;
	quiet_mode = DEFAULT_NO_LOGGING;
	verbose_mode = 0;
	no_detect = 0;
	bzero(last_ip, sizeof(last_ip));
	bzero(current_ip, sizeof(current_ip));
	conf_file = DEFAULT_CONF_FILE;
	check_period = DEFAULT_CHECK_PERIOD;
	update_error_count = 0;


	/* parse command line switches */
	for (i = 1; i < argc; ++i) {

		if(strcmp("-c",argv[i]) == 0) {
			if (++i == argc) {
				help();
				exit(EXIT_FAILURE);
			}
			conf_file = argv[i];
			continue;
		}

		if(strcmp("-a",argv[i]) == 0) {
			++i;
			/* make sure supplied IP is in correct format */
			if (sscanf(argv[i], "%u.%u.%u.%u", &int_tmp, &int_tmp, &int_tmp, &int_tmp) != 4) {
				printf("Invalid IP address. Program aborted.");
				exit(EXIT_FAILURE);
			} else {
				strncpy(current_ip, argv[i], IPADDR_LEN);
				no_detect = 1;
				continue;
			}
		}

		if(strcmp("--md5",argv[i]) == 0) {
                        if (++i == argc) {
                                help();
                                exit(EXIT_FAILURE);
                        }
			print_md5digest(argv[i]);
			exit(EXIT_SUCCESS);
		}

		else if (strcmp("-d",argv[i]) == 0) {
			daemon_mode = 1;
			continue;
		}
		else if (strcmp("-q",argv[i]) == 0) {
			quiet_mode = 1;
			continue;
		}
		else if (strcmp("-v",argv[i]) == 0) {
			verbose_mode = 1;
			continue;
		}
		else /* if (strcmp("-h",argv[i]) == 0) */ {
			help();
			exit(EXIT_SUCCESS);
		}
	}


	/* check for existing instance by reading pid file */
	if ((pid_fd = fopen(pid_file,"r")) != NULL) {
		printf("Another instance of miniupdate is running(pid file already exists). Program aborted.\n");
		exit(EXIT_FAILURE);
	}


	/* must alloc mem for path names before calling parseconf */
	//pid_file = (char *)malloc(MAX_FILE_PATH * sizeof(char));
	log_file = (char *)malloc(MAX_FILE_PATH * sizeof(char));


	if (parseconf(conf_file) == -1) {  /* fatal error */
		printf("Error parsing config file. Program aborted.\n");
		prg_exit(EXIT_FAILURE);
	}

	/* force non daemon mode if "-a" is used */
	daemon_mode = daemon_mode & (~ no_detect);

	/* quiet_mode (-q) also forces no_logging */
	no_logging = no_logging | quiet_mode;

	/* verbose mode in non daemon mode must be specified by -v switch */
	if (!daemon_mode) log_verbose = verbose_mode;


	/* open the log file if logging is enabled and not using syslog */
	if (!no_logging && !use_syslog) {
		if ((log_fd = fopen(log_file, "a")) == NULL) { /* fatal error */
			printf("Error opening log file. Program aborted.\n");
			prg_exit(1);
		}
	}


	/* log_file used, free mem */
	free(log_file);
	log_file = NULL;


	/* switch to the background */
	if (daemon_mode) {
		switch (pid = fork()) {
			case -1:
				perror("fork");
				exit(EXIT_FAILURE);
				break;
			case 0:
				//sleep(1);
				umask(022);
				if(setsid() < 0 ) {
					perror("setsid");
					exit(EXIT_FAILURE);
				}

				if ((chdir("/")) < 0) exit(EXIT_FAILURE);

				/* Daemon mode successful */
				break;

			default:
				/* parent exit here */
				exit(EXIT_SUCCESS);
				break;
		}
	}


	/* and write the pid to pid file */
	if ((pid_fd = fopen(pid_file,"w")) == NULL) {
		printf("Error opening pid file. Program aborted.\n");
		prg_exit(EXIT_FAILURE);
	} else {
		fprintf(pid_fd, "%d\n", getpid());
		fclose(pid_fd);
	}


	/* special setup for daemon mode */
	if (daemon_mode) {

	/* stop output to screen, use logwrite() from now on */
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		/* initialise signal handler */
		bzero(&action, sizeof(action));
		action.sa_handler = sigterm_handler;
		action.sa_flags = 0;
		sigaction(SIGTERM, &action, 0);
		sigaction(SIGINT, &action, 0);
	}

	/* if using syslog */
	if (!no_logging && use_syslog) {
		openlog("miniupdate", LOG_PID, LOG_USER);
	}


	/* annouce startup! */
	if (daemon_mode) {
		sprintf(msg, "*** %s v%s started ***\n", PROG_NAME, VERSION);
		logwrite(msg);
		if (log_fd != NULL) fflush(log_fd);
	}

	/* endless loop */
	for (;;) {

		/* get my ip */
		if (!no_detect) {

			if (no_public_ip) {
				if (get_myip(current_ip) < 0) logwrite("Failed to detect my IP using myip server");

			} else if (getifaddr(ifname, current_ip) < 0) {
				sprintf(msg, "Failed to get the IP of %s", ifname);
				logwrite(msg);
			}
		}

		/* do update if ip changed */
		if ((*current_ip != '\0') && strcmp(current_ip, last_ip)) {

			/* run script on IP changed */
			if ((strlen(on_change) > 0) && file_exists(on_change)) {
				ret = system(on_change);
			}

			if(ns_update(current_ip) == 0) {
				sprintf(msg, "%s mapped to %s", hostname, current_ip);
				logwrite(msg);
				strncpy(last_ip, current_ip, IPADDR_LEN);
				update_error_count = 0;
				check_period = DEFAULT_CHECK_PERIOD;

			} else {
				++update_error_count;

	        	if ((update_error_count > 0) && (update_error_count < 5)) {
            		check_period = BACKOFF_INTERVAL1;

				} else if ((update_error_count >= 5) && (update_error_count < 10)) {
            		check_period = BACKOFF_INTERVAL2;

				} else if (update_error_count >= 10) {
            		check_period = BACKOFF_INTERVAL3;
				}

				if (log_verbose) {
					sprintf(msg, "update_error_count:(%d), check_period set to:(%d)\n", update_error_count, check_period);
					logwrite(msg);
				}

			}

		} else {
			if (log_verbose) logwrite("No new IP detected");
		}


		/* let us see what's in the log immediately */
		if (log_fd != NULL) fflush(log_fd);

		if (daemon_mode) {
			sleep(check_period);
			continue;
		} else {
			prg_exit(update_error_count);
		}
	}
}

/**
 * @Author: WangZengze & WangXutao
 * @date: 2019/6/10
 * @version: 8.0
 *
 *
 *
 */



#include	<stdio.h>
#include	<stdlib.h>
#include	<netdb.h>	/* getservbyname(), gethostbyname() */
#include	<errno.h>	/* for definition of errno */
#include    <string.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> 
#include <termios.h>

/* define macros*/
#define MAXBUF	1024
#define STDIN_FILENO	0
#define STDOUT_FILENO	1

/* define FTP reply code */
#define USERNAME	220
#define PASSWORD	331
#define LOGIN		230
#define PATHNAME	257
#define CLOSEDATA	226
#define ACTIONOK	250

char *host;
char *port;
char *rbuf;
char *wbuf, *wbuf1;
struct sockaddr_in servaddr;
struct sockaddr_in local;
char filename[100];
char kk1[30];
char kk2[30];
char rbuf1[500];
int portselect[7] = { 10, 20, 40, 50, 60, 70, 30 };
int portselectnum = 0;
int cliopen(char *host, char *port);
void strtosrv(char *str, char *host, char *port);
void cmd_tcp(int sockfd);
void ftp_list(int sockfd);
int ftp_get(int sck, char *pDownloadFileName_s);
int ftp_put(int sck, char *pUploadFileName_s);
int ftp_activeopen(int port);

int speedflag = 0;

int main(int argc, char *argv[]) {
	int fd;

	if (0 != argc - 2) {
		printf("%s\n", "missing <hostname>");
		exit(0);
	}

	host = argv[1];
	port = "21";

	rbuf = (char *) malloc(sizeof(500));
	wbuf = (char *) malloc(sizeof(500));
	wbuf1 = (char *) malloc(sizeof(500));

	fd = cliopen(host, port);

	cmd_tcp(fd);

	exit(0);
}


/* Establish a TCP connection from client to server*/
int cliopen(char *host, char *port) {
	int sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);//open a socket

	memset(&servaddr, 0, sizeof(struct sockaddr_in));//clear structure
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(port));
	servaddr.sin_addr.s_addr = inet_addr(host);//configuration

	if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
		perror("connect\n");
		return 3;
	}
	//connect tcp
	printf("connetc success1\n");
	return sockfd;
}

int ftp_activeopen(int port1) {
	int sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&local, 0, sizeof(struct sockaddr_in));
	local.sin_family = AF_INET;
	local.sin_port = htons(port1);
	local.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(sockfd, (struct sockaddr*) &local, sizeof(local));

	listen(sockfd, 5);

	int new_sockfd;
	new_sockfd = accept(sockfd, (struct sockaddr*) NULL, NULL);
	return new_sockfd;

}

void strtosrv(char *str, char *host, char *port) {
	int addr[6];
	sscanf(str, "%*[^(](%d,%d,%d,%d,%d,%d)", &addr[0], &addr[1], &addr[2],
			&addr[3], &addr[4], &addr[5]);
	bzero(host, strlen(host));
	bzero(port, strlen(port));
	sprintf(host, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
	int portnumber = addr[4] * 256 + addr[5];
	sprintf(port, "%d", portnumber);
}

void cmd_tcp(int sockfd) {//loop
	int maxfdp1, nread, nwrite, fd, replycode;
	char host[16];
	char port[6];
	fd_set rset;
	int listflag = 0;
	int putflag = 0;
	int getflag = 0;
	int activeflag = 0;
	FD_ZERO(&rset);
	maxfdp1 = sockfd + 1;//*check descriptors [0..sockfd]

	for (;;) {
		FD_SET(STDIN_FILENO, &rset);
		FD_SET(sockfd, &rset);

		if (select(maxfdp1, &rset, NULL, NULL, NULL) < 0)
			printf("select error\n");
		/* data to read on stdin */
		if (FD_ISSET(STDIN_FILENO, &rset)) {
			bzero(rbuf, MAXBUF);
			bzero(wbuf, MAXBUF);
			if ((nread = read(STDIN_FILENO, rbuf, MAXBUF)) < 0)
				printf("read error from stdin\n");
			nwrite = nread + 5;
			/* send username */
			if (replycode == USERNAME) {
				sprintf(wbuf, "USER %s", rbuf);
				if (write(sockfd, wbuf, nwrite) != nwrite)
					printf("write error\n");
			}

			if (replycode == PASSWORD) {

				sprintf(wbuf, "PASS %s", rbuf);
				if (write(sockfd, wbuf, nwrite) != nwrite)
					printf("write error\n");
			}
			 /* send command */
			if (replycode == LOGIN || replycode == CLOSEDATA
					|| replycode == PATHNAME || replycode == ACTIONOK) {
				/* ls - list files and directories*/
				if (strncmp(rbuf, "ls", 2) == 0) {
					if (activeflag == 1) {
						int zx = portselect[portselectnum];
						sprintf(wbuf, "%s%d%s", "PORT 10,128,215,70,", zx,
								",0\n");
						write(sockfd, wbuf, strlen(wbuf));
						listflag = 1;
						continue;
					} else {
						sprintf(wbuf, "%s", "PASV\n");
						write(sockfd, wbuf, 5);
						listflag = 1;
						continue;
					}
				}
				if (strncmp(rbuf, "put", 3) == 0) {
					sscanf(rbuf, "put %s", filename);

					if (activeflag == 1) {
						int zx = portselect[portselectnum];
						sprintf(wbuf, "%s%d%s", "PORT 10,128,215,70,", zx,
								",0\n");
						write(sockfd, wbuf, strlen(wbuf));
						putflag = 1;
						continue;
					} else {
						sprintf(wbuf, "%s", "PASV\n");
						printf("%s\n", filename);
						write(sockfd, wbuf, 5);
						putflag = 1;
						continue;
					}
				}
				if (strncmp(rbuf, "active", 6) == 0) {
					activeflag = 1;
					write(STDOUT_FILENO, rbuf, nread);
					continue;
				}

				if (strncmp(rbuf, "lowspeed", 8) == 0) {
					speedflag = 1;
					write(STDOUT_FILENO, rbuf, nread);
					continue;
				}
				if (strncmp(rbuf, "highspeed", 9) == 0) {
					speedflag = 0;
					write(STDOUT_FILENO, rbuf, nread);
					continue;
				}

				if (strncmp(rbuf, "passive", 7) == 0) {
					activeflag = 0;
					write(STDOUT_FILENO, rbuf, nread);
					continue;
				}

				if (strncmp(rbuf, "pwd", 3) == 0) {
					sprintf(wbuf, "%s", "PWD\n");
					write(sockfd, wbuf, 4);
					continue;
				}

				if (strncmp(rbuf, "binary", 6) == 0) {
					sprintf(wbuf, "%s", "TYPE I\n");
					write(sockfd, wbuf, 7);
					continue;
				}

				if (strncmp(rbuf, "ascii", 5) == 0) {
					sprintf(wbuf, "%s", "TYPE A\n");
					write(sockfd, wbuf, 7);
					continue;
				}

				if (strncmp(rbuf, "mkd", 3) == 0) {
					sprintf(wbuf, "%s", rbuf);
					write(sockfd, wbuf, nread);
					continue;
				}

				if (strncmp(rbuf, "get", 3) == 0) {
					if (activeflag == 1) {
						sscanf(rbuf, "get %s", filename);
						int zx = portselect[portselectnum];
						sprintf(wbuf, "%s%d%s", "PORT 10,128,215,70,", zx,
								",0\n");
						write(sockfd, wbuf, strlen(wbuf));
						getflag = 1;
						continue;
					} else {
						sprintf(wbuf, "%s", "PASV\n");
						sscanf(rbuf, "get %s", filename);
						printf("%s\n", filename);
						write(sockfd, wbuf, 5);
						getflag = 1;
						continue;
					}
				}

				if (strncmp(rbuf, "dele", 4) == 0) {
					sprintf(wbuf, "%s", rbuf);
					write(sockfd, wbuf, nread);
					continue;
				}

				if (strncmp(rbuf, "cwd", 3) == 0) {
					sprintf(wbuf, "%s", rbuf);
					write(sockfd, wbuf, nread);
					continue;
				}

				if (strncmp(rbuf, "rename", 6) == 0) {

					sscanf(rbuf, "rename %s %s", kk1, kk2);
					if (kk1 == '\0' || kk2 == '\0') {
						printf("%s", "missing");
						continue;
					}
					sprintf(wbuf, "RNFR %s\n", kk1);
					// bzero(kk1,MAXBUF);
					write(sockfd, wbuf, strlen(wbuf));
					continue;
				}

				if (strncmp(rbuf, "quit", 4) == 0) {
					sprintf(wbuf, "%s", "QUIT\n");
					write(sockfd, wbuf, 5);
					continue;
				}

				bzero(rbuf, MAXBUF);
				bzero(wbuf, MAXBUF);
			}
		}

		if (FD_ISSET(sockfd, &rset)) {
			bzero(rbuf, MAXBUF);
			bzero(wbuf, MAXBUF);
			if ((nread = recv(sockfd, rbuf, MAXBUF, 0)) < 0)
				printf("recv error\n");
			else if (nread == 0) {
				break;
			}

			if (strncmp(rbuf, "220", 3) == 0 || strncmp(rbuf, "530", 3) == 0) {
				strcat(rbuf, "your name: ");
				nread += 12;
				replycode = USERNAME;
				system("stty echo");
			}

			if (strncmp(rbuf, "331", 3) == 0) {
				strcat(rbuf, "password: ");
				nread += 12;
				replycode = PASSWORD;
				system("stty -echo");
			}

			if (strncmp(rbuf, "230", 3) == 0) {
				replycode = LOGIN;
				system("stty echo");
			}

			if (strncmp(rbuf, "200", 3) == 0) {
				if (listflag == 1) {
					sprintf(wbuf, "%s", "LIST\n");
					nwrite = 5;
					write(sockfd, wbuf, nwrite);
					int xz = 256 * portselect[portselectnum];
					portselectnum++;
					fd = ftp_activeopen(xz);
					nwrite = 0;
					bzero(wbuf, MAXBUF);
					ftp_list(fd);
					listflag = 0;
				} else if (putflag == 1) {
					sprintf(wbuf, "STOR %s\n", filename);
					write(sockfd, wbuf, strlen(wbuf));
					int xz = 256 * portselect[portselectnum];
					portselectnum++;
					fd = ftp_activeopen(xz);
					bzero(wbuf, MAXBUF);
					ftp_put(fd, filename);
					putflag = 0;

				} else if (getflag == 1) {
					sprintf(wbuf, "RETR %s\n", filename);
					write(sockfd, wbuf, strlen(wbuf));
					int xz = 256 * portselect[portselectnum];
					portselectnum++;
					fd = ftp_activeopen(xz);
					bzero(wbuf, MAXBUF);
					ftp_get(fd, filename);
					getflag = 0;

				}

			}

			if (strncmp(rbuf, "226", 3) == 0) {
				replycode = LOGIN;

			}

			if (strncmp(rbuf, "550", 3) == 0) {
				close(fd);
				printf("the file is not exist");
			}

			if (strncmp(rbuf, "350", 3) == 0) {

				sprintf(wbuf, "RNTO %s\n", kk2);
				write(sockfd, wbuf, strlen(wbuf));

			}
			/* open data connection*/
			if (strncmp(rbuf, "227", 3) == 0) {

				strtosrv(rbuf, host, port);

				fd = cliopen(host, port);

				if (listflag == 1) {
					sprintf(wbuf, "%s", "LIST\n");
					nwrite = 5;
					write(sockfd, wbuf, nwrite);
					nwrite = 0;
					bzero(wbuf, MAXBUF);
					ftp_list(fd);
					listflag = 0;

				} else if (putflag == 1) {
					int checkhandle = open(filename, O_RDWR);
					if (checkhandle < 0) {
						close(fd);
						printf("the file is not exist");
						continue;
					}
					sprintf(wbuf, "STOR %s\n", filename);
					printf("%s\n", wbuf);
					write(sockfd, wbuf, strlen(wbuf));
					ftp_put(fd, filename);
					putflag = 0;

				} else if (getflag == 1) {
					sprintf(wbuf, "RETR %s\n", filename);
					write(sockfd, wbuf, strlen(wbuf));
					ftp_get(fd, filename);
					getflag = 0;
				}

			}
			if (write(STDOUT_FILENO, rbuf, nread) != nread)
				printf("write error to stdout\n");

			bzero(rbuf, MAXBUF);
			bzero(wbuf, MAXBUF);
		}
	}

	if (close(sockfd) < 0)
		printf("close error\n");
}

void ftp_list(int sockfd) {
	int nread = 0;

	for (;;) {
		if ((nread = recv(sockfd, rbuf1, MAXBUF, 0)) < 0)
			printf("recv error\n");
		else if (nread == 0)
			break;
		if (speedflag == 1) {
			sleep(1);
		}

		if (write(STDOUT_FILENO, rbuf1, nread) != nread)
			printf("send error to stdout\n");
	}
	bzero(rbuf1, MAXBUF);
	if (close(sockfd) < 0)
		printf("close error\n");
}

/* download file from ftp server */

int ftp_get(int sck, char *pDownloadFileName) {
	int handle = open(pDownloadFileName, O_RDWR | O_CREAT | O_APPEND,
	S_IREAD | S_IWRITE);
	int nread;
	printf("%d\n", handle);
	int i;
	i = lseek(handle, 0, SEEK_END);
	int k;
	k = i / 1024;
	for (;;) {
		if ((nread = recv(sck, rbuf1, MAXBUF, 0)) < 0) {
			printf("receive error\n");
		} else if (nread == 0) {
			printf("over\n");
			break;
		}
		if (speedflag == 1) {
			sleep(1);
		}
		if (k < 1) {
			if (write(handle, rbuf1, nread) != nread)
				printf("receive error from server!");

			if (write(STDOUT_FILENO, rbuf1, nread) != nread)
				printf("receive error from server!");
		}
		k--;

	}
	if (close(sck) < 0)
		printf("close error\n");
}

/* upload file to ftp server */
int ftp_put(int sck, char *pUploadFileName_s) {
	int handle = open(pUploadFileName_s, O_RDWR);
	int nread;
	bzero(rbuf1, MAXBUF);
	for (;;) {
		if ((nread = read(handle, rbuf1, MAXBUF)) < 0) {
			printf("read error!");
		} else if (nread == 0)
			break;
		if (speedflag == 1) {
			sleep(1);
		}

		if (write(STDOUT_FILENO, rbuf1, nread) != nread)
			printf("send error!");

		if (write(sck, rbuf1, nread) != nread)
			printf("send error!");
		bzero(rbuf1, MAXBUF);
	}
	if (close(sck) < 0)
		printf("close error\n");
}


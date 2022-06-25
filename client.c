/*
	CLIENT Program

	@author Samarth Saxena
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <unistd.h>
// #include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
// #include <errno.h>

#define PORT 8080
#define SIZE 1024

// extern int errno;

struct process
{
	char pid[10];
	char name[100];
	int cputime;
}topProcess;

int maxPID, clientNo;
char *request = "5";
char filename[20];


void receive_file(int sockfd)
{
	int n,i=0,lines = atoi(request);
	FILE *fp;
	char data[SIZE];
	char cmd[100];

	fp = fopen(filename, "w");
	// printf("[+] Receiving file...\n");

	// int recvVal = recv(sockfd, data, SIZE, 0);
	// if(recvVal<0)
	// {
	//     perror("[-] File receiving error");
	// }
	while(i<lines && recv(sockfd, data, SIZE, 0))
	{
		fprintf(fp, "%s", data);
		// printf("    [+] Received line %d:%s\n",i+1,data);
		// int recvVal = recv(sockfd, data, SIZE, 0);
		i++;
	}

	fclose(fp);

	printf("[+] File received:\n");
	sprintf(cmd, "cat %s", filename);
	system(cmd);

	return;
}

int getMaxPid()
{
	int fd;
	char buff[15];

	fd = open("/proc/sys/kernel/pid_max", O_RDONLY);
	read(fd,buff, 15);
	close(fd);

	return atoi(buff);
}

void printProcess(struct process p)
{
	printf("%s %s %d\n", p.pid, p.name, p.cputime);
	return;
}

void findProcess()
{
	int n,i=1;
	char data[SIZE] = {0};
	char path[50];

	int proc_fd;
	// int maxPID;
	// maxPID = getMaxPid();
	// struct process topProcess;


	while(i<maxPID)
	{
		struct process curProcess;
		curProcess.cputime = 0;
		sprintf(path, "/proc/%d/stat", i);


		proc_fd = open(path, O_RDONLY);

		if(proc_fd>=0)
		{
			read(proc_fd,data, SIZE);
			char *val = strtok (data, " ()");

			for (size_t i = 1; i < 16 && val!=NULL; i++)
			{
				if(i==1)
				{
					strcpy(curProcess.pid, val);
					val = strtok (NULL, "()");
				}
				else
				{
					if( i==2 )
					{
						strcpy(curProcess.name, val);
					}
					if( i== 14 )
					{
						curProcess.cputime = atoi(val);
					}
					if( i==15)
					{
						curProcess.cputime += atoi(val);
					}
					val = strtok (NULL, " ");
				}
			}

			if(curProcess.cputime>topProcess.cputime)
			{
				strcpy(topProcess.pid, curProcess.pid);
				strcpy(topProcess.name, curProcess.name);
				topProcess.cputime = curProcess.cputime;
			}

			if(close(proc_fd)<0)
			{
				perror("[-] Proc close error");
				exit(1);
			}
			// sleep(0.5);
		}


		i++;
	}
	printf("[+] Top process found: ");
	printProcess(topProcess);
	// return topProcess;
	return;
}

int main(int argc, char const *argv[])
{
	int sockfd, valread;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	char buffer[SIZE] = {0};
	char *hello = "Hello from client";
	char message[100];

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("[-] Socket creation failed");
		exit(1);
	}
	else {
		printf("[+] Socket created\n");
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons(PORT);

	if( connect(sockfd, (struct sockaddr *) &address, addrlen)<0)
	{
		perror("[-] Connection failed");
		exit(1);
	}
	else {
		printf("[+] Connection successful\n");
	}

	maxPID = getMaxPid();

	char strClientNo[10];
	recv (sockfd, strClientNo, 10,0);
	clientNo = atoi(strClientNo);
	printf("    Client no: %d\n", clientNo);
	sprintf(filename, "recvd_info_%d.txt", clientNo);

	// strcat(filename, strClientNo);
	// strcat(filename, ".txt");


	send(sockfd, request, strlen(request), 0);
	printf("\n[+] Requested: Top %s processes\n", request);

	receive_file(sockfd);


	findProcess();
	sprintf(message, "%s %s %d", topProcess.pid, topProcess.name, topProcess.cputime);

	send(sockfd, message, strlen(message), 0);
	printf("[+] Sent: \"%s\"\n", message);

	// valread = recv (sockfd, buffer, SIZE,0);
	// printf("[+] Received: \"%s\"\n", buffer);

	// fwrite();

	if(close(sockfd)<0)
	{
		perror("[-] Socket close failed");
		exit(1);
	}
	else {
		printf("\n[+] Socket closed\n");
	}

	return 0;
}



	// char *proc = "/proc/";
	// char pid[10];
	// char *stat = "/stat";

		// sprintf(pid, "%d", i);
		// printf("[+] PID updated: %d, %d\n",i,pid);

		// strcpy(path, proc);
		// printf("[+] Path = /proc\n");

		// strcat(path, pid);
		// printf("[+] Path = /proc/PID\n");

		// strcat(path, stat);
		// printf("[+] Path = %s\n", path);
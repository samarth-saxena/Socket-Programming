/*
	SERVER Program

	Assignment 1

	Samarth Saxena
	Roll no: 2019328
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/sendfile.h>


#define PORT 8080
#define SIZE 1024

int maxPID;
char *filename = "server_info.txt";


struct process
{
	char pid[10];
	char name[100];
	int cputime;
} processList[4194304];


int getMaxPid()
{
	int fd;
	char buff[15];

	fd = open("/proc/sys/kernel/pid_max", O_RDONLY);
	read(fd,buff, 15);
	close(fd);

	return atoi(buff);
}

int comparator(const void *p, const void *q)
{
	int l = ((struct process *)p)->cputime;
	int r = ((struct process *)q)->cputime;
	return (r-l);
}

void printProcess(struct process p)
{
	printf("%s %s %d\n", p.pid, p.name, p.cputime);
	return;
}

void writeNprocesses (int N, char* filename)
{
	int n;
	FILE *fp;
	// char *filename = "server_info.txt";
	char p_info[100];

	fp = fopen(filename, "w");
	for (int i = 0; i < N; i++)
	{
		sprintf(p_info, "%s %s %d", processList[i].pid, processList[i].name, processList[i].cputime);
		fprintf(fp, "%s\n", p_info);
	}
	printf("[+] Top processes written\n");

	fclose(fp);

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

			// if(curProcess.cputime>topProcess.cputime)
			// {
			//     strcpy(topProcess.pid, curProcess.pid);
			//     strcpy(topProcess.name, curProcess.name);
			//     topProcess.cputime = curProcess.cputime;
			// }

			strcpy(processList[i-1].pid, curProcess.pid);
			strcpy(processList[i-1].name, curProcess.name);
			processList[i-1].cputime = curProcess.cputime;

			if(close(proc_fd)<0)
			{
				perror("[-] Proc close error");
				exit(1);
			}
		}


		i++;
	}
	int size = sizeof(processList) / sizeof(processList[0]);
	qsort((void*)processList, size, sizeof(struct process), comparator);

	printf("[+] Processes sorted\n");
	// for (int i = 0; i < 1000; i++)
	// {
	// 	printf("i=%d ",i+1);
	// 	printProcess(processList[i]);
	// }
	// return topProcess;
	return;
}

int main(int argc, char const *argv[])
{
	int sockfd, new_sockfd, valread;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	char buffer[SIZE] = {0};
	int one = 1;
	char *hello = "Hello from server";
	char message[100], strN[10];
	int N;


	FILE *fp;

	// Socket setup
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("[-] Socket failed");
		exit(1);
	}
	else {
		printf("[+] Socket created\n");
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &one, sizeof(one)))
	{
		perror("[-] Socket options failed");
		exit(1);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	//Binding
	if(bind(sockfd, (struct sockaddr *)&address, addrlen) <0)
	{
		perror("[-] Bind failed");
		exit(1);
	}
	else {
		printf("[+] Bind successful\n");
	}

	//Listening
	if( listen(sockfd,5) < 0 )
	{
		perror("[-] Listen failed");
		exit(1);
	}
	else{
		printf("[+] Listening...\n");
	}

	maxPID = getMaxPid();

	//Accepting connection
	if((new_sockfd = accept(sockfd, (struct sockaddr *) &address, (socklen_t *) &addrlen) ) < 0)
	{
		perror("[-] Accept failed");
		exit(1);
	}
	else {
		printf("[+] Connection accepted\n");
	}

	//Receiving request for N proceses
	valread = recv (new_sockfd, strN, 10, 0);
	N = atoi(strN);
	printf("[+] Received: %d\n", N);

	findProcess();
	writeNprocesses(N, filename);


	//Sending file
	fp = fopen(filename, "r");
	if (fp == NULL)
	{
	    perror("[-] File opening error");
	}
	else
	{
	    // send_file(fp, sockfd);
	    int n, i=1;
	    char data[SIZE] = {0};

	    printf("[+] Sending file...\n");

	    while(fgets(data,SIZE, fp)!=NULL)
	    {

	        if(send(new_sockfd, data, sizeof(data),0) == -1)
	        {
	            perror("[-] Sending failed");
	            exit(1);
	        }
	        else {
	            printf("    [+] Sent line %d\n", i++);
	        }
	    }
	    printf("[+] File sent\n");

	}

	
	//Receiving client message
	valread = recv (new_sockfd, buffer, SIZE,0);
	printf("[+] Received: \"%s\"\n", buffer);

	if(close(sockfd)<0)
	{
		perror("[-] Socket close failed");
		exit(1);
	}
	else {
		printf("[+] Socket closed\n");
	}
	return 0;
}



	// int fp2 = open(filename, O_RDONLY);
	// if (fp2 <0) 
	// {
	// 	perror("[-] File reading error");
	// 	exit(1);
	// }

  	// sendfile(sockfd, fp2, NULL, BUFSIZ);
	// send_file(fp, new_sockfd);
	// printf("[+] File sent\n");


	// //10 CPU processes
	// system("ps -eo pid,cmd,%cpu --sort=-%cpu | head -n 11 >cpuinfo.txt");

	// //Sending file
	// fp = fopen(filename, "r");
	// if (fp == NULL)
	// {
	//     perror("[-] File opening error");
	// }
	// else
	// {
	//     // send_file(fp, sockfd);
	//     int n;
	//     char data[SIZE] = {0};

	//     //TODO add line counter
	//     printf("[+] File started\n");

	//     while(fgets(data,SIZE, fp)!=NULL)
	//     {

	//         if(send(new_sockfd, data, sizeof(data),0) == -1)
	//         {
	//             perror("[-] Sending failed");
	//             exit(1);
	//         }
	//         else {
	//             printf("[+] Sent line ...\n");
	//         }
	//     }
	//     printf("[+] File sent\n");

	// }
/*
	SERVER Program

	@author Samarth Saxena
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <pthread.h>


#define PORT 8080
#define SIZE 1024

int maxPID;
char *filename = "server_info.txt";
pthread_mutex_t lock;
int tcount=0;



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

void writeNprocesses (int N, char* filename, int myCLientNo)
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
	printf("[+] (%d) Top processes written\n", myCLientNo);

	fclose(fp);

	return;
}

void findProcess(int myCLientNo)
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

	printf("\n[+] (%d) Processes sorted\n", myCLientNo);
	// for (int i = 0; i < 1000; i++)
	// {
	// 	printf("i=%d ",i+1);
	// 	printProcess(processList[i]);
	// }
	// return topProcess;
	return;
}


void * clientThread (void * arg) //, void * arg2
{
	// int* myTID = (int*) arg1;
	int mySockFD = *((int*) arg);
	// arg++;
	int myClientNo = tcount;

	int recvVal;
	char *hello = "Hello from server";
	char strN[10];
	char buffer[SIZE] = {0};
	int N;
	char cmd[100];
	char strClientNo[10];

	sprintf(strClientNo, "%d", myClientNo);
	send(mySockFD, strClientNo, strlen(strClientNo), 0);
	// printf("[+] Sent: Client no %s \n", strClientNo);


	//Receiving request for N proceses
	recvVal = recv (mySockFD, strN, 10, 0);
	N = atoi(strN);
	printf("[+] (%d) Received request: %d\n", myClientNo, N);

	pthread_mutex_lock(&lock);

	findProcess(myClientNo);
	writeNprocesses(N, filename, myClientNo);

	sprintf(cmd, "cat %s", filename);
	system(cmd);


	//Sending file
	FILE *fp = fopen(filename, "r");
	if (fp == NULL)
	{
	    perror("[-] File opening error");
	}
	else
	{
	    // send_file(fp, sockfd);
	    int n, i=1;
	    char data[SIZE] = {0};

	    // printf("[+] Sending file...\n");

	    while(fgets(data,SIZE, fp)!=NULL)
	    {

	        if(send(mySockFD, data, sizeof(data),0) == -1)
	        {
	            perror("[-] Sending failed");
	            exit(1);
	        }
	        // else {
	        //     printf("    [+] Sent line %d\n", i++);
	        // }
	    }
		fclose(fp);
	    printf("[+] (%d) File sent\n", myClientNo);

	}
	pthread_mutex_unlock(&lock);

	
	//Receiving client message
	recvVal = recv (mySockFD, buffer, SIZE,0);
	printf("[+] (%d) Received: \"%s\"\n", myClientNo,buffer);

	if(close(mySockFD)<0)
	{
		printf("[-] (%d) Client socket close failed", myClientNo);
		exit(1);
	}
	else {
		printf("[+] (%d) Client socket closed\n\n", myClientNo);
	}
	pthread_exit(NULL);

}


int main(int argc, char const *argv[])
{
	int sockfd, new_sockfd;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	int one = 1;

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

	if (pthread_mutex_init(&lock, NULL) != 0) {
		perror("[-] Mutex init failed");
        exit(1);
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
	pthread_t tid[10];

	while((new_sockfd = accept(sockfd, (struct sockaddr *) &address, (socklen_t *) &addrlen) ) >= 0)
	{

		//Accepting connection
		// if((new_sockfd = accept(sockfd, (struct sockaddr *) &address, (socklen_t *) &addrlen) ) < 0)
		// {
		// 	perror("[-] Accept failed");
		// 	exit(1);
		// }
		// else {
			printf("\n[+] Connection accepted\n");
			// int arr[2], *p;
			// arr[0] = new_sockfd;
			// arr[1] = tcount;
			// p = arr;
			//&new_sockfd

			if( pthread_create(&tid[tcount], NULL, clientThread, &new_sockfd) != 0 )
			{
        		printf("Failed to create thread\n");
			}
			else {
				printf("[+] Client %d thread created\n", ++tcount );
			}
			// tcount++;
		// }

	}

	//pthread_join(tid[],NULL);
	// pthread_mutex_destroy(&lock);

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
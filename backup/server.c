/*
    SERVER Socket Program

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


#define PORT 8080
#define SIZE 1024

struct process 
{
    char pid[10];
    char name[100];
    int cputime;
} topProcess,processList[4194304];

int maxPID;

// void send_file(FILE *fp, int sockfd)
// {
//     int n;
//     char data[SIZE] = {0};

//     //TODO add line counter
//     printf("File ended");

//     while(fgets(data,SIZE, fp)!=NULL)
//     {
//         if(send(sockfd, data, sizeof(data),0) == -1)
//         {
//             perror("[-] Sending failed");
//             exit(1);
//         }
//         else {
//             printf("[+] Sent line ...");
//         }
//     }
//     return;
// }

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
    int sockfd, new_sockfd, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[SIZE] = {0};
    int one = 1;
    char *hello = "Hello from server";
    char message[100];


    FILE *fp;
    char *filename = "cpuinfo.txt";

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
        printf("[+] Accepted\n");
    }


    valread = recv (new_sockfd, buffer, SIZE,0);
    printf("[+] Received: %d\n", atoi(buffer));

    findProcess();
    // struct process p = findProcess(getMaxPid());
    sprintf(message, "%s %s %d", topProcess.pid, topProcess.name, topProcess.cputime);
    send(new_sockfd, message, strlen(message), 0);
    printf("[+] Sent: \"%s\"\n", message);


    valread = recv (new_sockfd, buffer, SIZE,0);
    printf("[+] Received: \"%s\"\n", buffer);
    // send(new_sockfd, hello, strlen(hello), 0);
    // printf("[+] Message sent");

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
#include  <unistd.h>
#include  <sys/types.h>       /* basic system data types */
#include  <sys/socket.h>      /* basic socket definitions */
#include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include  <arpa/inet.h>       /* inet(3) functions */

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define MAXLINE 1024
//typedef struct sockaddr  SA;
typedef struct _recvmodel
{
        int sk_connect;
        struct sockaddr_in addr;
}RecvModel;

void handle(int connfd,struct sockaddr_in addr);

//Linux Send Thread

void  *send_thread(void * arg)
{
    if (arg==NULL)
    {   
        printf("send_thread param is not allow NULL!\n");
        return NULL;
    }

    printf("******** Send_thread is running now *******\n");

    RecvModel model = *((RecvModel *) arg);;


    char buf[1024]={0};
    while(1)
    {
        read(STDIN_FILENO,buf,sizeof(buf));
        if (send(model.sk_connect,buf,strlen(buf),0)==-1)
        {
            printf("Send failed ! error message %s\n",strerror(errno));
            return NULL;
        }
        memset(buf,0,sizeof(buf));
    }
    return NULL;
}

//recv message
void * recv_thread(void * arg)
{
    if (arg == NULL)
    {
        printf("Recv_thread param is not allow NULL!\n");
        return NULL;
    }

    printf("-------- Recv_thread is running now ------\n");

    RecvModel model = *((RecvModel *) arg);
    int flag = 0;
    char buf[1024] = { 0 };
    while (1)
    {
        flag = recv(model.sk_connect, buf, sizeof(buf), 0);
        if (flag == 0)
        {
            printf("对方已经关闭连接！\n");
            return NULL;
        } else if (flag == -1)
        {
            printf("recv failed ! error message : %s\n", strerror(errno));
            return NULL;
        }

        printf("Recv_thread---recv msg from Host: %s:%d ---- Data:%s\n",inet_ntoa(model.addr.sin_addr), model.addr.sin_port, buf);

        memset(buf, 0, sizeof(buf));
    }
    return NULL;
}


void handle(int connfd,struct sockaddr_in addr);

int  main(int argc, char **argv)
{
    int  listenfd, connfd;
    int  serverPort = 6888;
    int listenq = 1024;
    pid_t   childpid;
    char buf[MAXLINE];
    socklen_t socklen;

    struct sockaddr_in cliaddr, servaddr;
    socklen = sizeof(cliaddr);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(serverPort);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket error");
        return -1;
    }
    if (bind(listenfd, (struct sockaddr *) &servaddr, socklen) < 0) {
        perror("bind error");
        return -1;
    }
    if (listen(listenfd, listenq) < 0) {
        perror("listen error");    
        return -1;
    }
    printf("echo server startup,listen on port:%d\n", serverPort);
    for ( ; ; )  {
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &socklen);
        if (connfd < 0) {
            perror("accept error");
            continue;
        }

        sprintf(buf, "accept form %s:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
        printf(buf,"");
        printf("Host will begin to run the fork function \n");
        childpid = fork();
        printf("the childpid number is:%d\n", childpid);
        if (childpid == 0) { /* child process */
            close(listenfd);    /* close listening socket */
            handle(connfd,cliaddr);   /* process the request */
            exit (0);
        } else if (childpid > 0)  {
            close(connfd);          /* parent closes connected socket */
        } else {
            perror("fork error");
        }
    }
}


void handle(int connfd,struct sockaddr_in addr)
{
    size_t n;
    char    buf[MAXLINE];
    for(;;) {
        n = read(connfd, buf, MAXLINE);
        if (n < 0) {
            if(errno != EINTR) {
                perror("read error");
                break;
            }
        }
        if (n == 0) {
            //connfd is closed by client
            close(connfd);
            printf("client exit\n");
            break;
        }
        //client exit
        if (strncmp("exit", buf, 4) == 0) {
            close(connfd);
            printf("client exit\n");
            break;
        }
        buf[n] = '\0';  
        printf("recv msg from client: %s:%d ---- Data:%s\n",inet_ntoa(addr.sin_addr), addr.sin_port, buf);
        write(connfd, buf, n); //write maybe fail,here don't process failed error
    } 
} 
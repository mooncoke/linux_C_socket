#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
  
#define MAXLINE 1024  

typedef struct _recvmodel
{
		int sk_connect;
		struct sockaddr_in addr;
}RecvModel;

typedef struct _mesmodel
{
        char userid;
        char data[100];
}MesModel;

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
    MesModel  tmp ;

	char buf[100]={0};
	while(1)
	{      
        printf("please input the USERID whihc your message will be delivered !!!\n");
		read(STDIN_FILENO,&tmp.userid,sizeof(&tmp.userid));
 
        printf("please input the Data whihc your message will be send to the USERID !!!\n");
        read(STDIN_FILENO,&tmp.data,sizeof(&tmp.data));

		if (send(model.sk_connect,&tmp,strlen(&tmp),0)==-1)
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
        //Socket Buffer maybe longer than buf siza.
        //check the first char to find the clinet name:
        printf("this message comes from the User:%c --", buf[0]);

        printf("Recv_thread---recv msg from Host: %s:%d -- Data:%s\n",inet_ntoa(model.addr.sin_addr), model.addr.sin_port, buf);

        memset(buf, 0, sizeof(buf));
    }
    return NULL;
}


//Handle function
void handle(int connfd,struct sockaddr_in addr)
{
    RecvModel model;
    model.sk_connect= connfd;
    model.addr = addr;
    pthread_t thr1, thr2;
    if (pthread_create(&thr1, NULL, send_thread, (void *)&model) != 0)
    {
        printf("create thread failed ! \n");
        close(connfd);
        exit(0);

    }
    sleep(3);
    if (pthread_create(&thr2, NULL, recv_thread, (void *)&model) != 0)
    {
        printf("create thread failed ! \n");
        close(connfd);
        exit(0);

    }
    pthread_join(thr1, NULL);
    printf("send_thread pthread join Pass \n");
    pthread_join(thr2, NULL);
    printf("Recv_thread pthread join Pass \n");
}

    

void main(int argc,char *argv[])
{

        char tmp[100];
        FILE *fp;
        int i;
        int n,rec,len;
        char recvline[4096],sendline[4096];
        char buf[MAXLINE];

        // fp=fopen("hello.txt","rw");
        // if(fp==NULL)
        // {
        //         printf("Open the hello.txt file failed !!!\n");
        //         exit(1);
        // }

        // if(fread(tmp,sizeof(char),10,fp)!=10)
        // {
        //         if(feof(fp))
        //                 printf("End of file");
        //         else
        //                 printf("Read error");
        // }

  //       if(argc !=2)
  //       {
  //               printf("usage:./client <ipaddress\n>");
  //       }

  //       if((connfd=socket(AF_INET,SOCK_STREAM,0))<0)
  //       {
  //               printf("create socket error:%s(error:%d)\n",strerror(errno),errno);
  //               exit(0);
  //       }
		// //bzero(&servaddr, sizeof(servaddr));
		// memset(&servaddr, 0, sizeof(servaddr));  
		// servaddr.sin_family = AF_INET;  
		// servaddr.sin_port = htons(8000);  
		// if( inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0){  
		// printf("inet_pton error for %s\n",argv[1]);  
		// exit(0);  
		// }  
	  
	  
		// if( connect(connfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){  
		// printf("connect error: %s(errno: %d)\n",strerror(errno),errno); 
		// goto END;
		 
		// }  


    char * servInetAddr = "127.0.0.1";
    int servPort = 6888;
    int connfd;
    struct sockaddr_in servaddr;

    if (argc == 2) {
        servInetAddr = argv[1];
    }
    if (argc == 3) {
        servInetAddr = argv[1];
        servPort = atoi(argv[2]);
    }
    if (argc > 3) {
        printf("usage: echoclient <IPaddress> <Port>\n");
        return -1;
    }

    connfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(servPort);
    inet_pton(AF_INET, servInetAddr, &servaddr.sin_addr);

    if (connect(connfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        return -1;
    }

    //----------------Connect Pass--------

    printf("welcome to echoclient\n");
    handle(connfd,servaddr);     /* do it all */

    //开启多线程--线程1接收消息，线程2发送消息

    END: close(connfd);
    return 0;	  
	  
	  
	/*   
		printf("send msg to server: \n");  
		fgets(sendline, 4096, stdin);  
		if( send(connfd, sendline, strlen(sendline), 0) < 0)  
		{  
		printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);  
		exit(0);  
		}  
		if((rec_len = recv(connfd, buf, MAXLINE,0)) == -1) {  
		   perror("recv error");  
		   exit(1);  
		}  
		buf[rec_len]  = '\0';  
		printf("Received : %s ",buf);  
		close(connfd);  
		exit(0);   */

}


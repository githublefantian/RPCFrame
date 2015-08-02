#include <myunp.h>
using namespace std;

int main()
{
    //**********************数据定义**********************
    int listenfd;//套接字描述符
    sockaddr_in  servaddr,cliaddr;//服务器和客户端网络地址
    pthread_t childThread;//用于创建子线程
    socklen_t	 clilen;//soket长度
    cli_message *tcon;//
    //****************************************************
    creatAndSetSock(listenfd,servaddr);//设置监听套接字
    while(1)//主循环
    {
        clilen = sizeof(cliaddr);
        bzero(&cliaddr, clilen);
        tcon=(cli_message *)malloc(sizeof(cli_message));
        //阻塞于此处，等待返回一个新的tcp连接
        tcon->clifd=accept(listenfd,(SA *) &cliaddr,&clilen);
        tcon->cliaddr=cliaddr;
        if (tcon->clifd == -1)//错误处理
        {
            if ((errno == EAGAIN) ||    //队列中没有可用的连接
                    (errno == EWOULDBLOCK))     //没有阻塞成功
            {
                printf("accept return a EAGAIN or EWOULDBLOCK erro.\n");
                break;
            }
            else
            {
                printf("accept return a unknown erro.\n");
                break;
            }
        }
        //创建子线程处理连接
        pthread_create(&childThread,NULL,&connect_worker,tcon);
    }

    return 0;
}

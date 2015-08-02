#ifndef MYUNP_H_INCLUDED
#define MYUNP_H_INCLUDED
#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<errno.h>
#include<sys/epoll.h>
#include<sys/select.h>
#include<fcntl.h>
#include<pthread.h>
#include<map>
#include<iostream>
#include"x.pb.h"

using namespace std;
using namespace x;

//define
#define	SA	                        struct sockaddr
#define	MAXLINE		        4096	/* max text line length */
#define	LISTENQ		        1024	/* 2nd argument to listen() */
#define     MAXEVENTS        64
//函数声明
ssize_t	 readn(int, void *, size_t);
ssize_t	 writen(int, const void *, size_t);
ssize_t	 my_read(int , char *);
ssize_t	 readline(int, void *, size_t);
int make_socket_non_blocking (int sfd);
void creatAndSetSock(int &listenfd,sockaddr_in &servaddr);
void *connect_worker(void *arg);
int connect_to_server(int &sock, struct sockaddr_in &servaddr);
//静态变量
static int	read_cnt;
static char	*read_ptr;
static char	read_buf[MAXLINE];
//常量
const string SERV_IP = "192.168.1.103";/* TCP and UDP client-servers */
const int SERV_PORT = 5566;
//结构体声明
struct cli_message
{
    int clifd;
    sockaddr_in cliaddr;
};
//函数声明

ssize_t						/* Read "n" bytes from a descriptor. */
readn(int fd, void *vptr, size_t n)
{
    size_t	nleft;
    ssize_t	nread;
    char	*ptr;

    ptr = (char *)vptr;
    nleft = n;
    while (nleft > 0)
    {
        if ( (nread = read(fd, ptr, nleft)) < 0)
        {
            if (errno == EINTR)
                nread = 0;		/* and call read() again */
            else
                return(-1);
        }
        else if (nread == 0)
            break;				/* EOF */

        nleft -= nread;
        ptr   += nread;
    }
    return(n - nleft);		/* return >= 0 */
}
/* end readn */

ssize_t						/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
    size_t		nleft;
    ssize_t		nwritten;
    const char	*ptr;

    ptr = (char *)vptr;
    nleft = n;
    while (nleft > 0)
    {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;		/* and call write() again */
            else
                return(-1);			/* error */
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(n);
}
/* end writen */

ssize_t
my_read(int fd, char *ptr)
{

    if (read_cnt <= 0)
    {
again:
        if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0)
        {
            if (errno == EINTR)
                goto again;
            return(-1);
        }
        else if (read_cnt == 0)
            return(0);
        read_ptr = read_buf;
    }

    read_cnt--;
    *ptr = *read_ptr++;
    return(1);
}

ssize_t
readline(int fd, void *vptr, size_t maxlen)
{
    size_t	n, rc;
    char	c, *ptr;

    ptr = (char *)vptr;
    for (n = 1; n < maxlen; n++)
    {
        if ( (rc = read(fd, &c,1)) == 1)
        {
            if (c == '\n')
                break;	/* skip '\n' and break */
            *(ptr++) = c;
        }
        else if (rc == 0)
        {
            *ptr = 0;
            return(n - 1);	/* EOF, n - 1 bytes were read */
        }
        else
            return(-1);		/* error, errno set by read() */
    }

    *ptr = 0;	/* null terminate like fgets() */
    return(n);
}

//设置socket为非阻塞的
int make_socket_non_blocking (int sfd)
{

    int flags, s;
    //得到文件状态标志
    flags = fcntl (sfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror ("fcntl");
        return -1;
    }
    //设置文件状态标志
    flags |= O_NONBLOCK;
    //设置新的状态
    s = fcntl (sfd, F_SETFL, flags);
    if (s == -1)
    {
        perror ("fcntl");
        return -1;
    }
    return 0;
}

//初始化一个监听套接字
void creatAndSetSock(int &listenfd,sockaddr_in &servaddr)
{
    listenfd = socket(AF_INET, SOCK_STREAM, 0);//设置socket
    bzero(&servaddr, sizeof(servaddr));//初始化地址结构体为0
    servaddr.sin_family = AF_INET;//协议是ipv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//设置地址
    servaddr.sin_port = htons(SERV_PORT);//设置端口
    bind(listenfd, (SA *) &servaddr, sizeof(servaddr));//绑定套接字和地址
    listen(listenfd, LISTENQ);//开启监听，但是没有调用accept来从监听到的表中提取连接
}

//线程处理函数
void *connect_worker(void *arg)
{
    //从函数参数中获取数据，文件描述符和客户端地址
    int fd=(*(cli_message *)arg).clifd,n;
    sockaddr_in cliaddr=(*(cli_message *)arg).cliaddr;
    //数据缓冲区
    char buf[MAXLINE];
    //释放传递来的参数内存
    free(arg);
    //解除线程关系
    pthread_detach(pthread_self());
    cout<<"new user connectted!"<<endl;
    memset(buf, 0, sizeof(buf));
    if((n = readline(fd, buf, MAXLINE)) > 0)
    {
        buf[n] = '\0';
        string  data= buf;
        person p;
        p.ParseFromString(data);
        cout<<"*********************"<<endl;
        cout<<"username:"<<p.name()<<endl;
        cout<<"id:"<<p.id()<<endl;
        cout<<"email:"<<p.email()<<endl;
        cout<<"*********************"<<endl;
    }
    close(fd);
    return 0;
}

//客户端连接初始化
int connect_to_server(int &sockfd, struct sockaddr_in &servaddr)
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;//协议是ipv4
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, SERV_IP.c_str(), &servaddr.sin_addr);
    connect(sockfd, (SA *)&servaddr, sizeof(servaddr));
    return sockfd;
}

#endif // MYUNP_H_INCLUDED



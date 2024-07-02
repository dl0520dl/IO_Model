#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>
using namespace std;


int initserver(int port);

int main(int argc,char *argv[])
{
    if(argc != 2)
    {
        perror("failed");
        return -1;

    }
    int listenfd = initserver(atoi(argv[1]));
    if(listenfd < 0)
    {
        perror("failed");
        return -1;
    }
    cout << "listenfd is " << listenfd << endl;

    // 创建epoll句柄。
    int epollfd=epoll_create(1);

    epoll_event ev;         //声明事件的数据结构
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;    //打算让epoll监视listensock的读事件

    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);  //把需要监视的socket和事件加入epollfd中

    epoll_event evs[10];

    while (1)
    {
        int infds = epoll_wait(epollfd, evs, 10, -1);

        if(infds < 0)
        {
            perror("epoll() faild");
            break;
        }
        if(infds == 0)
        {
            perror("epoll() timeout");
            continue;
        }

        for(int i = 0; i < infds; i++)
        {
            if(evs[i].data.fd == listenfd)
            {
                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int clientfd = accept(listenfd, (struct sockaddr*)&clientaddr, &len);
                if(clientfd < 0)
                {
                    perror("accept() failed");
                    continue;
                }
                ev.data.fd = clientfd;
                ev.events = EPOLLIN;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev);   
            }
            else
            {
                char buffer[1024];
                memset(buffer, 0, sizeof(buffer));
                if(recv(evs[i].data.fd, buffer, sizeof(buffer), 0) <= 0)
                {
                    cout << "disconnect" << evs[i].data.fd << endl;
                    close(evs[i].data.fd);
                    //epoll_ctl(epollfd, EPOLL_CTL_DEL, evs[i].data.fd, 0);
                }
                else
                {
                    cout << "recv:" << buffer << endl;
                    send(evs[i].data.fd, buffer, strlen(buffer), 0);
                }

            }
        }

    }
    return 0;

}

int initserver(int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("create socket failed");
        return -1;
    }
    int on = 1;
    unsigned int len = sizeof(on);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, len);
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind() failed");
        close(sockfd);
        return -1;
    }
    if(listen(sockfd, 5) != 0)
    {
        perror("listen failed");
        close(sockfd);
        return -1;
    }
    return sockfd;
}
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
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

    pollfd fds[2048];
    for(int i = 0; i < 2048; i++)
    {
        fds[i].fd = -1;
    }
    fds[listenfd].fd = listenfd;
    fds[listenfd].events = POLLIN;

    int maxfd = listenfd;
    
    while (1)
    {
        int infds = poll(fds, maxfd + 1, 10000);

        if(infds < 0)
        {
            perror("poll() faild");
            break;
        }
        if(infds == 0)
        {
            perror("poll() timeout");
            continue;
        }

        for(int eventfd = 0; eventfd <= maxfd; eventfd++)
        {
            if(fds[eventfd].fd < 0)
            {
                continue;
            }
            if((fds[eventfd].revents&POLLIN) == 0)
            {
                continue;
            }
            if(eventfd == listenfd)
            {
                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int clientfd = accept(listenfd, (struct sockaddr*)&clientaddr, &len);
                if(clientfd < 0)
                {
                    perror("accept() failed");
                    continue;
                }
                fds[clientfd].fd = clientfd;
                fds[clientfd].events = POLLIN;
                if(maxfd < clientfd)
                {
                    maxfd = clientfd;
                }
            }
            else
            {
                char buffer[1024];
                memset(buffer, 0, sizeof(buffer));
                if(recv(eventfd, buffer, sizeof(buffer), 0) <= 0)
                {
                    cout << "disconnect" << eventfd << endl;
                    close(eventfd);
                    fds[eventfd].fd = -1;

                    if(eventfd == maxfd)
                    {
                        for(int ii = maxfd; ii > 0; ii--)
                        {
                            if(fds[ii].fd != -1)
                            {
                                maxfd = ii; 
                                break;
                            }
                        }
                    }

                }
                else
                {
                    cout << "recv:" << buffer << endl;
                    send(eventfd, buffer, strlen(buffer), 0);
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
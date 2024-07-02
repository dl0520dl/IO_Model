#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
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

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(listenfd, &readfds);
    int maxfd = listenfd;

    while (1)
    {
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        fd_set tmpfds = readfds;

        int infds = select(maxfd + 1, &tmpfds, NULL, NULL, 0);

        if(infds < 0)
        {
            perror("select() faild");
            break;
        }
        if(infds == 0)
        {
            perror("select() timeout");
            continue;
        }
        for(int eventfd = 0; eventfd <= maxfd; eventfd++)
        {
            if(FD_ISSET(eventfd, &tmpfds) == 0)
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
                FD_SET(clientfd, &readfds);
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
                    cout << "disconnect:" << eventfd << endl;
                    close(eventfd);
                    FD_CLR(eventfd, &readfds);

                    if(eventfd == maxfd)
                    {
                        for(int ii = maxfd; ii > 0; ii--)
                        {
                            if(FD_ISSET(ii, &readfds))
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
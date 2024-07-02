#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
using namespace std;



int main(int argc,char *argv[])
{
    if(argc != 3)
    {
        perror("failed");
        return -1;

    }
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("create socket failed");
        return -1;
    }
    struct sockaddr_in servaddr;
    char buffer[1024];
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        cout << "connect() faild" << endl;
        return -1;
    }
    cout << "connect() ok" << endl;

    for(int i = 0; i < 20000; i++)
    {
        memset(buffer, 0, sizeof(buffer));
        cout << "input msg:";
        cin >> buffer;
        if(send(sockfd, buffer, strlen(buffer), 0) <= 0)
        {
            cout << "send() failed" << endl;
            close(sockfd);
            return -1;
        }
        memset(buffer, 0, sizeof(buffer));
        if(recv(sockfd, buffer, sizeof(buffer), 0) <= 0)
        {
            cout << "recv() failed" << endl;
            close(sockfd);
            return -1;
        }
        cout << "receive:" << buffer << endl;
    }

}
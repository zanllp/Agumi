#include "../stdafx.h"
#include "../Event.h"

namespace sion
{

    void error(const char *msg)
    {
        perror(msg);
        exit(1);
    }

    int MakeServer(int portno, agumi::ServerHandler handler)
    {
        int sockfd;
        socklen_t clilen;
        sockaddr_in serv_addr, cli_addr;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
            error("ERROR opening socket");
        bzero((char *)&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        if (bind(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            error("ERROR on binding");
        listen(sockfd, 50);
        clilen = sizeof(cli_addr);
        while (true)
        {
            int newsockfd = accept(sockfd, (sockaddr *)&cli_addr, &clilen);
            if (newsockfd < 0)
                error("ERROR on accept");
            auto msg_handler = [=]
            {
                while (true)
                {
                    char buf[256];
                    bzero(buf, 256);
                    int n = read(newsockfd, buf, 255);
                    if (n < 0)
                        error("ERROR reading from socket");
                    agumi::ServerRecvEvent recv_e;
                    recv_e.event_name = "ServerRecvEvent";
                    recv_e.val = buf;
                    recv_e.fd = newsockfd;
                    if (!handler.on_recv(recv_e))
                    {
                        close(newsockfd);
                        return;
                    }
                }
            };
            std::thread t(msg_handler);
            t.detach();
        }

        close(sockfd);
        return 0;
    }

}
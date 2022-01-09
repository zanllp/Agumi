#pragma once
#include "../Event.h"
#include "../stdafx.h"

namespace sion
{

void setNonBlock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    agumi::Assest(flags >= 0, "fcntl failed");
    int r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    agumi::Assest(r >= 0, "fcntl failed");
}

void error(const char* msg, int socket = -1)
{
    ::close(socket);
    perror(msg);
    throw std::runtime_error("");
}

std::atomic<long> MakeServer_connection_incr_id = 0;
/*
 * 先随便写写能跑就行，后面再换成epoll/kqueue/iocp
 */
int MakeServer(int portno, agumi::ServerHandler handler)
{
    auto unsafe_tid = agumi::get_thread_id();
    int sockfd;
    socklen_t clilen;
    sockaddr_in serv_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 50);
    clilen = sizeof(cli_addr);
    handler.on_init(portno, unsafe_tid);
    while (true)
    {
        int newsockfd = accept(sockfd, (sockaddr*)&cli_addr, &clilen);
        MakeServer_connection_incr_id += 1;
        double connection_id = MakeServer_connection_incr_id.load();
        auto message = handler.on_channel_message(unsafe_tid);
        for (auto&& i : message)
        {
            if (i.event_name == "close")
            {
                close(newsockfd);
                close(sockfd);
                return -1;
            }
        }

        if (newsockfd < 0)
            error("ERROR on accept");
        setNonBlock(newsockfd);
        auto msg_handler = [=] {
            auto unsafe_tid = agumi::get_thread_id();

            while (true)
            {
                auto message = handler.on_channel_message(unsafe_tid);
                for (auto&& i : message)
                {
                    if (i.event_name == "send_data")
                    {
                        auto data = i.val.ToString();
                        auto ee = write(newsockfd, data.c_str(), data.size());
                    }
                    else if (i.event_name == "close_connection")
                    {
                        ::close(newsockfd);
                        return;
                    }
                }
                const auto size = 1024; 
                char buf[size];
                bzero(buf, size);
                int n = read(newsockfd, buf, size -1);
                if (n < 0)
                {
                    if (errno == EAGAIN || errno == EINPROGRESS)
                    {
                        continue;
                    }
                    error("ERROR reading from socket", newsockfd);
                }
                if (n == 0)
                {
                    std::this_thread::yield();
                    continue;
                }
                agumi::ServerRecvEvent recv_e;
                recv_e.event_name = "ServerRecvEvent";
                recv_e.val = buf;
                recv_e.fd = newsockfd;
                recv_e.tid_unsafe = unsafe_tid;
                recv_e.connection_id = connection_id;
                handler.on_recv(recv_e);
            }
        };
        std::thread t(msg_handler);
        t.detach();
    }

    close(sockfd);
    return 0;
}

} // namespace sion
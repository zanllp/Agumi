#pragma once
#include "../Event.h"
#include "../ResouceControl.h"
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
    agumi::resource_control.FreeSocket(socket);
    perror(msg);
    throw std::runtime_error("");
}

std::atomic<long> MakeServer_connection_incr_id = 0;
/*
 * 先随便写写能跑就行，后面再换成epoll kqueue/iocp
 */
int MakeServer(int portno, agumi::ServerHandler handler)
{
    auto unsafe_tid = agumi::get_thread_id();
    socklen_t clilen;
    sockaddr_in serv_addr, cli_addr;
    int sockfd = agumi::resource_control.CreateSocket();
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
        agumi::resource_control.RecordSocket(newsockfd);
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, (sockaddr*)&cli_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
        MakeServer_connection_incr_id += 1;
        double connection_id = MakeServer_connection_incr_id.load();
        auto message = handler.on_channel_message(unsafe_tid);
        for (auto&& i : message)
        {
            if (i.event_name == "close")
            {
                agumi::resource_control.FreeSocket(newsockfd);
                agumi::resource_control.FreeSocket(sockfd);
                return -1;
            }
        }

        if (newsockfd < 0)
            error("ERROR on accept");
        setNonBlock(newsockfd);
        auto msg_handler = [=] {
            auto unsafe_tid = agumi::get_thread_id();
            int message_incr_id = 0;
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
                        agumi::resource_control.FreeSocket(newsockfd);
                        return;
                    }
                }
                const auto size = 256;
                char buf[size];
                bzero(buf, size);
                int n = read(newsockfd, buf, size - 1);
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
                recv_e.message_id = ++message_incr_id;
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

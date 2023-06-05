
#pragma once
#include "stdafx.h"
namespace agumi
{

class ResouceControl
{
  private:
    std::mutex scoket_m;
    std::set<int> sockets;

  public:
    ResouceControl(/* args */) {}
    ~ResouceControl()
    {
        {
            std::lock_guard<std::mutex> m(scoket_m);
            for (auto&& i : sockets)
            {
                P("close socket:{} : res: {}",i, close(i));
            }
            sockets.clear();
        }
    }
    void FreeSocket(int socket)
    {
        std::lock_guard<std::mutex> m(scoket_m);
        close(socket);
        sockets.erase(socket);
    }
    int RecordSocket(int socket)
    {
        std::lock_guard<std::mutex> m(scoket_m);
        sockets.insert(socket);
        return socket;
    }
    int CreateSocket(int __domain = AF_INET, int __type = SOCK_STREAM, int __protocol = 0)
    {
        return RecordSocket(socket(__domain, __type, __protocol));
    }
};

ResouceControl resource_control;
} // namespace agumi

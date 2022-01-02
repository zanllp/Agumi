#pragma once

#include "util.h"

namespace agumi
{
    struct BaseEvent
    {
        String event_name;
        Value val;
    };

    /*
     * 必要事件处理完之前不允许退出
     */
    struct RequiredEvent : BaseEvent
    {
    };

    struct CrossThreadEvent : BaseEvent
    {
    };

    struct ServerRecvEvent : BaseEvent
    {
        int fd;
    };

    struct ServerHandler
    {
        std::function<bool(ServerRecvEvent)> on_recv;
    };
    
}

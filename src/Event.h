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

    struct CrossThreadCallBack
    {
        Value cb;
        CrossThreadEvent event;
    };

    struct ServerRecvEvent : BaseEvent
    {
        int fd;
        double tid_unsafe; // 仅用于比较，map
    };

    struct ChannelPayload  : BaseEvent
    {
        int id;
    };
    

    struct ServerHandler
    {
        using ChannelPayloadArr = Vector<ChannelPayload>;
        std::function<bool(ServerRecvEvent)> on_recv;
        std::function<ChannelPayloadArr(double)> on_channel_message;
    };
    
}

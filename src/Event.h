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
        RequiredEvent() = default;
        RequiredEvent(String name, Value v = Value::null)
        {
            event_name = name;
            val = v;
        }
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
        int connection_id;
        int message_id;
        double tid_unsafe; // 仅用于比较，map，发布消息
    };

    struct ChannelPayload  : BaseEvent
    {
        int id;
    };
    

    struct ServerHandler
    {
        using ChannelPayloadArr = Vector<ChannelPayload>;
        std::function<bool(ServerRecvEvent)> on_recv;
        std::function<void(int, double)> on_init;
        std::function<ChannelPayloadArr(double)> on_channel_message;
    };
    
}

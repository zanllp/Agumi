#pragma once
#include "Array.h"
#include "Json.h"
#include "Object.h"
#include "Runtime.h"
#include "Value.h"
#include "sion/server.h"

#define CLONE(v) JSON_PARSE(Json::Stringify(v))

#define VM_FN_BIND(fn) std::bind(fn, std::ref(vm), std::placeholders::_1)

namespace agumi
{

Value SendServerDataBind(VM& vm, Vector<Value> args)
{
    auto arg = args.GetOrDefault(0);
    ChannelPayload payload;
    payload.event_name = "send_data";
    payload.val = args.GetOrDefault(1);
    vm.ChannelPublish(arg["#tid_unsafe"].Number(), payload);
    return nullptr;
}

Value AddServerMessageCallback(VM& vm, Vector<Value> args)
{
    auto arg = args.GetOrDefault(0);
    ChannelPayload payload;
    payload.event_name = "send_data";
    payload.val = args.GetOrDefault(1);
    vm.ChannelPublish(arg["#tid_unsafe"].Number(), payload);
    return nullptr;
}

Vector<ChannelPayload> ReceiveChannelPayload(VM& vm, double tid)
{
    std::lock_guard<std::mutex> m(vm.channel_mutex);
    if (vm.sub_thread_channel.find(tid) == vm.sub_thread_channel.end())
    {
        return {};
    }
    auto payload_queue = vm.sub_thread_channel[tid];
    vm.sub_thread_channel.erase(tid);
    return payload_queue;
};

class Server
{
  private:
    /* data */
  public:
    Server(/* args */);
    ~Server();
    static std::map<int, Value> connection;
};

Server::Server(/* args */) {}

Server::~Server() {}

std::map<int, Value> Server::connection;

Value MakeServerBind(VM& vm, Vector<Value> args)
{
    auto params = args.GetOrDefault(0).ObjC();
    auto port = params["port"].Number();
    auto onAccept = params["onAccept"];
    auto onInit = params["onInit"];
    static int id = 0;
    id++;
    String close_event_name = String::Format("make_server:close:{}", id);
    String init_event_name = String::Format("make_server:init:{}", id);
    vm.AddRequiredEventCustomer(close_event_name, [&](RequiredEvent e) {

    });
    vm.AddRequiredEventCustomer(init_event_name, [&, onInit](RequiredEvent e) {
        if (onInit.NotUndef())
        {
            vm.FuncCall(onInit, e.val);
        }
    });
    auto messageCb = vm.DefineFunc([&, onAccept](Vector<Value> args) {
        auto conn = args.GetOrDefault(0);
        int connection_id = conn["connection_id"].Number();
        int is_first_message = false;
        if (Server::connection.find(connection_id) == Server::connection.end())
        {
            if (onAccept.NotUndef())
            {
                vm.FuncCall(vm.GlobalVal("use_ability"), conn, vm.GlobalVal("ServerConnection"));
                vm.FuncCall(onAccept, conn);
                Server::connection[connection_id] = conn;
                is_first_message = true;
            }
        }

        auto conn_iter = Server::connection.find(connection_id);
        if (conn_iter->second.In("onMessage"))
        {
            if (!is_first_message)
            {
                conn_iter->second["buf"] = conn["buf"];
                conn_iter->second["message_id"] = conn["message_id"];
            }
            vm.FuncCall(conn_iter->second["onMessage"], conn_iter->second);
        }

        return nullptr;
    });
    std::thread t([&, close_event_name, init_event_name, port, messageCb, onInit] {
        ServerHandler sh;
        sh.on_recv = [&, messageCb](ServerRecvEvent e) {
            CrossThreadEvent cte;
            cte.val = Object({{"name", e.event_name},
                              {"buf", e.val},
                              {"#tid_unsafe", e.tid_unsafe},
                              {"connection_id", e.connection_id},
                              {"message_id", e.message_id}});
            cte.event_name = e.event_name;
            CrossThreadCallBack ctcb;
            ctcb.cb = messageCb;
            ctcb.event = cte;
            vm.Push2CrossThreadEventPendingQueue(ctcb);
            return false;
        };
        sh.on_channel_message = VM_FN_BIND(ReceiveChannelPayload);
        sh.on_init = [&](auto port, auto tid) {
            vm.Push2RequiredEventPendingQueue(RequiredEvent(init_event_name, Object({{"port", port}, {"#main_tid", tid}})));
        };
        sion::MakeServer(port, sh);
        vm.Push2RequiredEventPendingQueue(RequiredEvent(close_event_name));
    });
    t.detach();
    return nullptr;
}

void ServerBind(VM& vm)
{
    vm.DefineGlobalFunc("send_server_data", VM_FN_BIND(SendServerDataBind));
    vm.DefineGlobalFunc("add_server_message_callback", VM_FN_BIND(AddServerMessageCallback));
    vm.DefineGlobalFunc("close_server_connection", [&](Vector<Value> args) -> Value {
        auto arg = args.GetOrDefault(0);
        ChannelPayload payload;
        payload.event_name = "close_connection";
        vm.ChannelPublish(arg["#tid_unsafe"].Number(), payload);
        return nullptr;
    });
    vm.DefineGlobalFunc("make_server", VM_FN_BIND(MakeServerBind));
}
} // namespace agumi
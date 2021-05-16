#include "stdafx.h"
#pragma once

template <typename T>
class Future
{
private:
    T *res;

public:
    Future();
    ~Future();
    T &Res()
    {
        return *this->res;
    }
    bool IsCompleted()
    {
        return false;
    }
};
std::queue<std::function<void()>> event_pool;
std::queue<Future<void>> await_pool;
std::queue<std::function<void()>> event_pool;
#define ASYNC(block) event_pool.push([]() { block });
#define ASYNC_ref(block) event_pool.push([&]() { block; });
#define ASYNC_cp(block) event_pool.push([=]() { block; });
#define IIFE (block, r) [&] { block;return r;};
#define AWAIT(r) IIFE(await_pool.push(r), await_pool.back())

void RunEventLoop()
{
    while (!event_pool.empty())
    {
        auto front = event_pool.front();
        event_pool.pop();
        front();

    }
}

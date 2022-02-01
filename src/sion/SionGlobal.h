#pragma once
#include "./sion.h"

class SionGlobal
{
public:
  static sion::Async async_thread_pool;
};

sion::Async SionGlobal::async_thread_pool;

#pragma once
#include "stdafx.h"
#include "JsValue.h"
#include "JsObject.h"
using namespace std;

class MemAllocCollect
{
public:
    static Vector<const JsObjectMap *> obj_quene;
    static Vector<const JsArrayVec *> vec_quene;
};

class MemManger
{
private:
    static MemManger *mem;

public:
    MemManger();
    ~MemManger();
    JsValue gc_root;
    bool first;
    set<const JsObjectMap *> can_reach_obj;
    set<const JsArrayVec *> can_reach_arr;
    void ReachObjectNode(JsObject start);
    void GC();
    static MemManger &Get();
};
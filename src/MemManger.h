#pragma once
#include "stdafx.h"
#include "Value.h"
#include "JsObject.h"
namespace agumi
{
    

    class MemAllocCollect
    {
    public:
        static Vector<const JsObjectMap *> obj_quene;
        static Vector<const ArrayVec *> vec_quene;
    };

    class MemManger
    {
    private:
        static MemManger *mem;

    public:
        MemManger();
        ~MemManger();
        Value gc_root;
        bool first;
        std::set<const JsObjectMap *> can_reach_obj;
        std::set<const ArrayVec *> can_reach_arr;
        void ReachObjectNode(JsObject start);
        void GC();
        static MemManger &Get();
    };
}
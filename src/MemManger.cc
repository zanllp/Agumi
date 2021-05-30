#include "MemManger.h"
#include "Object.h"
#include "stdafx.h"
namespace agumi
{

    MemManger::MemManger() : gc_root(Object()) {}
    MemManger::~MemManger() {}

    Vector<const ObjectMap *> MemAllocCollect::obj_quene = {};
    Vector<const ArrayVec *> MemAllocCollect::vec_quene = {};
    MemManger *MemManger::mem = nullptr;
    MemManger &MemManger::Get()
    {
        if (mem == nullptr)
        {
            mem = new MemManger();
        }
        return *mem;
    }

    void MemManger::ReachObjectNode(Object start)
    {
        std::vector<Object> obj_set = {start};
        std::vector<Array> arr_set;
        while (obj_set.size() + arr_set.size())
        {
            if (obj_set.size())
            {
                auto obj = obj_set.back();
                obj_set.pop_back();
                for (const auto &i : obj.Src())
                {
                    auto next = i.second;
                    if (next.Type() == JsType::object && can_reach_obj.find(next.Obj().Ptr()) == can_reach_obj.end())
                    {
                        auto object_node = next.Obj();
                        can_reach_obj.insert(object_node.Ptr());
                        obj_set.push_back(object_node);
                    }
                    else if (next.Type() == JsType::array && can_reach_arr.find(next.Arr().Ptr()) == can_reach_arr.end())
                    {
                        auto arr_node = next.Arr();
                        can_reach_arr.insert(arr_node.Ptr());
                        arr_set.push_back(arr_node);
                    }
                }
            }
            else if (arr_set.size())
            {
                auto arr = arr_set.back();
                arr_set.pop_back();
                for (const auto &i : arr.Src())
                {
                    auto next = i;
                    if (next.Type() == JsType::array && can_reach_arr.find(next.Arr().Ptr()) == can_reach_arr.end())
                    {
                        auto arr_node = next.Arr();
                        can_reach_arr.insert(arr_node.Ptr());
                        arr_set.push_back(arr_node);
                    }
                    else if (next.Type() == JsType::object && can_reach_obj.find(next.Obj().Ptr()) == can_reach_obj.end())
                    {
                        auto object_node = next.Obj();
                        can_reach_obj.insert(object_node.Ptr());
                        obj_set.push_back(object_node);
                    }
                }
            }
        }
    }

    void MemManger::GC()
    {
        can_reach_obj.insert(gc_root.Obj().Ptr());
        ReachObjectNode(gc_root.Obj());
        for (auto i : MemAllocCollect::obj_quene)
        {
            if (can_reach_obj.find(i) == can_reach_obj.end())
            {
                delete i;
            }
        }
        for (auto i : MemAllocCollect::vec_quene)
        {
            if (can_reach_arr.find(i) == can_reach_arr.end())
            {
                delete i;
            }
        }
        MemAllocCollect::vec_quene.resize(can_reach_arr.size());
        MemAllocCollect::obj_quene.resize(can_reach_obj.size());
        copy(can_reach_arr.begin(), can_reach_arr.end(), MemAllocCollect::vec_quene.begin());
        copy(can_reach_obj.begin(), can_reach_obj.end(), MemAllocCollect::obj_quene.begin());
        can_reach_obj.clear();
        can_reach_arr.clear();
#ifdef JS_RUNTIME_TEST
        cout << "GC完成  对象数量：" << MemAllocCollect::obj_quene.size() << "   "
             << "数组数量：" << MemAllocCollect::vec_quene.size() << endl;
#endif
    }
}
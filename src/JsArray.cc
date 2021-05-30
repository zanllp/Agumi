#include "JsArray.h"
#include "JsValue.h"
#include "stdafx.h"
#include "MemManger.h"
namespace agumi
{
    JsArray::JsArray()
    {
        data_ptr = new JsArrayVec();
        MemAllocCollect::vec_quene.push_back(data_ptr);
    }
    JsArray::JsArray(std::initializer_list<Value> arg)
    {
        data_ptr = new JsArrayVec(arg);
        MemAllocCollect::vec_quene.push_back(data_ptr);
    }
    Value &JsArray::operator[](int key)
    {
        return (*data_ptr)[key];
    }
    JsArrayVec &JsArray::Src()
    {
        return *data_ptr;
    }

    const JsArrayVec *JsArray::Ptr() const
    {
        return this->data_ptr;
    }

    const JsArrayVec &JsArray::SrcC() const
    {
        return *data_ptr;
    }
    bool JsArray::In(size_t idx) const
    {
        return this->SrcC().size() > idx;
    }

    JsArray::~JsArray()
    {
    }
}
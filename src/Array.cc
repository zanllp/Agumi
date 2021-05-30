#include "Array.h"
#include "Value.h"
#include "stdafx.h"
#include "MemManger.h"
namespace agumi
{
    Array::Array()
    {
        data_ptr = new ArrayVec();
        MemAllocCollect::vec_quene.push_back(data_ptr);
    }
    Array::Array(std::initializer_list<Value> arg)
    {
        data_ptr = new ArrayVec(arg);
        MemAllocCollect::vec_quene.push_back(data_ptr);
    }
    Value &Array::operator[](int key)
    {
        return (*data_ptr)[key];
    }
    ArrayVec &Array::Src()
    {
        return *data_ptr;
    }

    const ArrayVec *Array::Ptr() const
    {
        return this->data_ptr;
    }

    const ArrayVec &Array::SrcC() const
    {
        return *data_ptr;
    }
    bool Array::In(size_t idx) const
    {
        return this->SrcC().size() > idx;
    }

    Array::~Array()
    {
    }
}
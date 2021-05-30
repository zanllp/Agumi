#pragma once
#include "stdafx.h"
namespace agumi
{
    class Value;
    class Array
    {
        ArrayVec *data_ptr;

    public:
        Array();
        Array(std::initializer_list<Value> arg);
        ~Array();
        const ArrayVec *Ptr() const;
        bool In(size_t idx) const;

        Value &operator[](int key);
        ArrayVec &Src();
        const ArrayVec &SrcC() const;
    };
}
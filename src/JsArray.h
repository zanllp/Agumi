#pragma once
#include "stdafx.h"
namespace agumi
{
    class Value;
    class JsArray
    {
        JsArrayVec *data_ptr;

    public:
        JsArray();
        JsArray(std::initializer_list<Value> arg);
        ~JsArray();
        const JsArrayVec *Ptr() const;
        bool In(size_t idx) const;

        Value &operator[](int key);
        JsArrayVec &Src();
        const JsArrayVec &SrcC() const;
    };
}
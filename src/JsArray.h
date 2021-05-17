#pragma once
#include "stdafx.h"
namespace agumi
{
    class JsValue;
    class JsArray
    {
        JsArrayVec *data_ptr;

    public:
        JsArray();
        JsArray(std::initializer_list<JsValue> arg);
        ~JsArray();
        const JsArrayVec *Ptr() const;
        bool In(size_t idx) const;

        JsValue &operator[](int key);
        JsArrayVec &Src();
        const JsArrayVec &SrcC() const;
    };
}
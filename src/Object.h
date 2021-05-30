#pragma once
#include "stdafx.h"
namespace agumi
{
    class Value;
    class Object
    {

        ObjectMap *data_ptr;

    public:
        Object();
        Object(std::initializer_list<std::pair<const String, Value>> arg);
        ~Object();
        const ObjectMap *Ptr() const;
        Value &operator[](std::string key);
        ObjectMap &Src();
        const ObjectMap &SrcC() const;
        bool In(String key) const;
    };
}
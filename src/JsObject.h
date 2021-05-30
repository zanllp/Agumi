#pragma once
#include "stdafx.h"
namespace agumi
{
    class Value;
    class JsObject
    {

        JsObjectMap *data_ptr;

    public:
        JsObject();
        JsObject(std::initializer_list<std::pair<const String, Value>> arg);
        ~JsObject();
        const JsObjectMap *Ptr() const;
        Value &operator[](std::string key);
        JsObjectMap &Src();
        const JsObjectMap &SrcC() const;
        bool In(String key) const;
    };
}
#pragma once
#include "stdafx.h"
class JsValue;
class JsObject
{

    JsObjectMap *data_ptr;
public:
    JsObject();
    JsObject(std::initializer_list<std::pair<const String, JsValue>> arg);
    ~JsObject();
    const JsObjectMap * Ptr() const;
    JsValue &operator[](std::string key);
    JsObjectMap &Src();
    const JsObjectMap &SrcC() const;
    bool In(String key) const;
};

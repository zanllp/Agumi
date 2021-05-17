#include "JsObject.h"
#include "JsValue.h"
#include "stdafx.h"
#include "MemManger.h"

namespace agumi
{
    JsObject::JsObject()
    {
        data_ptr = new JsObjectMap();
        MemAllocCollect::obj_quene.push_back(data_ptr);
    }
    JsObject::JsObject(std::initializer_list<std::pair<const String, JsValue>> arg)
    {
        data_ptr = new JsObjectMap(arg);
        MemAllocCollect::obj_quene.push_back(data_ptr);
    }
    JsValue &JsObject::operator[](std::string key)
    {
        return (*data_ptr)[key];
    }
    JsObjectMap &JsObject::Src()
    {
        return *data_ptr;
    }

    bool JsObject::In(String key) const
    {
        return this->SrcC().find(key) != this->SrcC().end();
    }

    const JsObjectMap &JsObject::SrcC() const
    {
        return *data_ptr;
    }

    const JsObjectMap *JsObject::Ptr() const
    {
        return this->data_ptr;
    }

    JsObject::~JsObject()
    {
    }
}
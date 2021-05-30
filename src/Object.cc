#include "Object.h"
#include "Value.h"
#include "stdafx.h"
#include "MemManger.h"

namespace agumi
{
    Object::Object()
    {
        data_ptr = new ObjectMap();
        MemAllocCollect::obj_quene.push_back(data_ptr);
    }
    Object::Object(std::initializer_list<std::pair<const String, Value>> arg)
    {
        data_ptr = new ObjectMap(arg);
        MemAllocCollect::obj_quene.push_back(data_ptr);
    }
    Value &Object::operator[](std::string key)
    {
        return (*data_ptr)[key];
    }
    ObjectMap &Object::Src()
    {
        return *data_ptr;
    }

    bool Object::In(String key) const
    {
        return this->SrcC().find(key) != this->SrcC().end();
    }

    const ObjectMap &Object::SrcC() const
    {
        return *data_ptr;
    }

    const ObjectMap *Object::Ptr() const
    {
        return this->data_ptr;
    }

    Object::~Object()
    {
    }
}
#include "util.h"
#include "Json.h"
#include "JsValue.h"
using namespace std;
JsValue::~JsValue()
{
    if (data_ptr != nullptr)
    {
        switch (type)
        {
        case JsType::number:
            delete (double *)data_ptr;
            break;
        case JsType::function:
        case JsType::string:
            delete (String *)data_ptr;
            break;
        case JsType::boolean:
            delete (bool *)data_ptr;
            break;
        case JsType::object:
            delete (JsObject *)data_ptr;
            break;
        case JsType::array:
            delete (JsArray *)data_ptr;
            break;
        case JsType::null:
        case JsType::undefined:
            break;
        }
    }
};

JsValue::JsValue()
{
}
JsValue::JsValue(const JsValue &v)
{
    type = v.type;
    switch (v.type)
    {
    case JsType::number:
        data_ptr = new double(*((double *)v.data_ptr));
        break;
    case JsType::function:
    case JsType::string:
        data_ptr = new String(*(String *)(v.data_ptr));
        break;
    case JsType::boolean:
        data_ptr = new bool(*(bool *)(v.data_ptr));
        break;
    case JsType::object:
        data_ptr = new JsObject(*(JsObject *)(v.data_ptr));
    case JsType::array:
        data_ptr = new JsArray(*(JsArray *)(v.data_ptr));
    case JsType::null:
    case JsType::undefined:
    default:
        break;
    }
}
JsValue::JsValue(JsValue &&v)
{
    swap(type, v.type);
    swap(data_ptr, v.data_ptr);
}
JsValue::JsValue(bool data)
{
    type = JsType::boolean;
    data_ptr = new bool(data);
}
JsValue::JsValue(const char *data)
{
    type = JsType::string;
    data_ptr = new String(data);
}
JsValue::JsValue(int data)
{
    type = JsType::number;
    data_ptr = new double(data);
}
JsValue::JsValue(nullptr_t null)
{
    type = JsType::null;
}
JsValue::JsValue(double data)
{
    type = JsType::number;
    data_ptr = new double(data);
}
JsValue::JsValue(string data)
{
    type = JsType::string;
    data_ptr = new String(data);
}

JsValue::JsValue(JsObject obj)
{
    type = JsType::object;
    data_ptr = new JsObject(obj);
}
JsValue::JsValue(JsArray arr)
{
    type = JsType::array;
    data_ptr = new JsArray(arr);
}
JsType JsValue::Type() const
{
    return type;
}

JsValue &JsValue::operator=(const JsValue &v)
{
    type = v.type;
    switch (v.type)
    {
    case JsType::number:
        data_ptr = new double(*((double *)v.data_ptr));
        break;
    case JsType::function:
    case JsType::string:
        data_ptr = new String(*(String *)(v.data_ptr));
        break;
    case JsType::boolean:
        data_ptr = new bool(*(bool *)(v.data_ptr)); // 传值
        break;
    case JsType::object:
        data_ptr = new JsObject(*(JsObject *)(v.data_ptr));
    case JsType::array:
        data_ptr = new JsArray(*(JsArray *)(v.data_ptr));
    case JsType::null:
    case JsType::undefined:
    default:
        break;
    }
    return *this;
}

JsValue &JsValue::operator[](String key)
{
    return Object()[key];
}

JsObject &JsValue::Object()
{
    if (type != JsType::object)
    {
        THROW_MSG("get object了一个不是Object的JsValue实例");
    }
    return *(JsObject *)data_ptr;
}

const JsObject &JsValue::ObjectC() const
{
    if (type != JsType::object)
    {
        THROW_MSG("get object了一个不是Object的JsValue实例");
    }
    return *(JsObject *)data_ptr;
}

JsValue &JsValue::operator[](int key)
{
    return Array()[key];
}
JsArray &JsValue::Array()
{
    if (type != JsType::array)
    {
        THROW_MSG("get array了一个不是array的JsValue实例");
    }
    return *(JsArray *)data_ptr;
}

bool JsValue::In(const String &key) const
{
    return this->ObjectC().In(key);
}
bool JsValue::In(size_t idx) const
{
    return this->ArrayC().In(idx);
}

const JsArray &JsValue::ArrayC() const
{
    if (type != JsType::array)
    {
        THROW_MSG("get array了一个不是array的JsValue实例");
    }
    return *(JsArray *)data_ptr;
}
bool JsValue::NotUndef()
{
    return type != JsType::undefined;
}

String JsValue::ToString() const
{
    switch (type)
    {
    case JsType::boolean:
        return GetC<bool>();
    case JsType::number:
        return GetC<double>();
    case JsType::string:
        return GetC<String>();
    case JsType::null:
        return "null";
    case JsType::undefined:
        return "undefined";
    case JsType::array:
        return "[Array]";
    case JsType::object:
        return "[Object]";
    case JsType::function:
        return String::Format("[Function loc:{}]", GetC<String>());
    }
}

bool JsValue::ToBool() const
{
    switch (type)
    {
    case JsType::boolean:
        return GetC<bool>();
    case JsType::number:
        return GetC<double>() != 0.0;
    case JsType::string:
        return GetC<String>().size() != 0;
    case JsType::null:
    case JsType::undefined:
        return false;
    case JsType::array:
    case JsType::function:
    case JsType::object:
        return true;
    }
    return false;
}

bool JsValue::DeepCompare(const JsValue &r) const
{
    const auto &l = *this;
    if (l.data_ptr == r.data_ptr)
    {
        return true;
    }
    auto type = l.Type();
    if (type != r.Type())
    {
        return false;
    }
    if (type == JsType::function)
    {
        return l.GetC<String>() == r.GetC<String>();
    }
    else if (type == JsType::object)
    {

        if (l.ObjectC().SrcC().size() != r.ObjectC().SrcC().size())
        {
            return false;
        }

        for (auto &i : l.ObjectC().SrcC())
        {
            if (!i.second.DeepCompare(r.ObjectC().SrcC().at(i.first)))
            {
                return false;
            }
        }
    }
    else if (type == JsType::array)
    {

        auto &larr = l.ArrayC().SrcC();
        auto &rarr = r.ArrayC().SrcC();
        if (larr.size() != rarr.size())
        {
            return false;
        }
        for (int i = larr.size() - 1; i >= 0; i--)
        {
            if (!larr[i].DeepCompare(rarr[i]))
            {
                return false;
            }
        }
    }
    else
    {
        return l.ToString() == r.ToString();
    }

    return true;
}

bool JsValue::operator==(const JsValue &rhs)
{
    return DeepCompare(rhs);
}
JsValue JsValue::undefined =  JsValue();
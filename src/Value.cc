#include "Value.h"
#include "JsonStringify.h"
#include "util.h"
namespace agumi
{
Value::~Value(){};

Value::Value() {}
Value::Value(const Value& v)
{
    type = v.type;
    val = v.val;
}
Value::Value(Value&& v)
{
    std::swap(type, v.type);
    std::swap(val, v.val);
}
Value::Value(bool data)
{
    type = ValueType::boolean;
    val = data;
}
Value::Value(const char* data)
{
    type = ValueType::string;
    val = String(data);
}
Value::Value(int data)
{
    type = ValueType::number;
    val = double(data);
}
Value::Value(std::nullptr_t null) { type = ValueType::null; }
Value::Value(double data)
{
    type = ValueType::number;
    val = data;
}
Value::Value(std::string data)
{
    type = ValueType::string;
    val = String(data);
}
Value::Value(String data)
{
    type = ValueType::string;
    val = data;
}

Value::Value(Object obj)
{
    type = ValueType::object;
    val = obj;
}
Value::Value(Array arr)
{
    type = ValueType::array;
    val = arr;
}
ValueType Value::Type() const { return type; }

Value& Value::operator=(const Value& v)
{
    type = v.type;
    val = v.val;
    return *this;
}

String Value::TypeString() const
{
    auto t = Type();
    return type_emun2str[static_cast<int>(t)];
}

Value& Value::operator[](String key)
{
    CheckType(ValueType::object);
    return Obj()[key];
}

Object& Value::Obj()
{
    CheckType(ValueType::object);
    return std::get<Object>(val);
}

const Object& Value::ObjC() const
{
    CheckType(ValueType::object);
    return std::get<Object>(val);
}

Value& Value::operator[](int key)
{
    CheckType(ValueType::array);
    return Arr()[key];
}
Array& Value::Arr()
{
    CheckType(ValueType::array);
    return std::get<Array>(val);
}

const Array& Value::ArrC() const
{
    CheckType(ValueType::array);
    return std::get<Array>(val);
}

bool Value::In(const String& key) const
{
    if (type != ValueType::object)
    {
        return false;
    }

    return this->ObjC().In(key);
}

bool Value::In(size_t idx) const
{
    if (type != ValueType::array)
    {
        return false;
    }
    return this->ArrC().In(idx);
}

bool Value::NotUndef() const { return type != ValueType::null; }

String Value::ToString() const
{
    switch (type)
    {
    case ValueType::boolean:
        return BoolC();
    case ValueType::number:
        return NumberC();
    case ValueType::string:
        return StrC();
    case ValueType::null:
        return "null";
    case ValueType::array:
        return "[Array]";
    case ValueType::object:
        return "[Object]";
    case ValueType::function:
        return String::Format("[Function id:{}]", StrC());
    }
}
double& Value::Number()
{
    CheckType(ValueType::number);
    return std::get<double>(val);
}
bool& Value::Bool()
{
    CheckType(ValueType::boolean);
    return std::get<bool>(val);
}
String& Value::Str()
{
    CheckType(ValueType::string);
    return std::get<String>(val);
}
const double& Value::NumberC() const
{
    CheckType(ValueType::number);
    return std::get<double>(val);
}
const bool& Value::BoolC() const
{
    CheckType(ValueType::boolean);
    return std::get<bool>(val);
}
const String& Value::StrC() const
{
    CheckType(ValueType::string);
    return std::get<String>(val);
}
void Value::CheckType(ValueType t) const
{
    if (type != t)
    {
        if (type == ValueType::function && t == ValueType::string)
        {
            return;
        }
        THROW_MSG("type error. expect '{}' reality '{}'", type_emun2str[(int)t], TypeString())
    }
}

bool Value::ToBool() const
{
    switch (type)
    {
    case ValueType::boolean:
        return BoolC();
    case ValueType::number:
        return NumberC() != 0.0;
    case ValueType::string:
        return StrC().size() != 0;
    case ValueType::null:
        return false;
    case ValueType::array:
    case ValueType::function:
    case ValueType::object:
        return true;
    }
    return false;
}

bool Value::DeepCompare(const Value& r) const
{
    const auto& l = *this;
    auto type = l.Type();
    auto r_t = r.type;
    if (type != r_t)
    {
        return false;
    }
    if (type == ValueType::function)
    {
        return l.StrC() == r.StrC();
    }
    else if (type == ValueType::object)
    {

        if (l.ObjC().Ptr() == r.ObjC().Ptr())
        {
            return true;
        }

        if (l.ObjC().SrcC().size() != r.ObjC().SrcC().size())
        {
            return false;
        }
        for (auto& i : l.ObjC().SrcC())
        {
            if (!i.second.DeepCompare(r.ObjC().SrcC().at(i.first)))
            {
                return false;
            }
        }
    }
    else if (type == ValueType::array)
    {

        auto& larr = l.ArrC().SrcC();
        auto& rarr = r.ArrC().SrcC();
        if (&larr == &rarr)
        {
            return true;
        }
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

bool Value::operator==(const Value& rhs) { return DeepCompare(rhs); }
Value Value::null = Value();
} // namespace agumi

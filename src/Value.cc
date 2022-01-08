#include "util.h"
#include "Json.h"
#include "Value.h"
namespace agumi
{
    Value::~Value()
    {
        if (data_ptr != nullptr)
        {
            switch (type)
            {
            case ValueType::number:
                delete (double *)data_ptr;
                break;
            case ValueType::function:
            case ValueType::string:
                delete (String *)data_ptr;
                break;
            case ValueType::boolean:
                delete (bool *)data_ptr;
                break;
            case ValueType::object:
                delete (Object *)data_ptr;
                break;
            case ValueType::array:
                delete (Array *)data_ptr;
                break;
            case ValueType::null:
                break;
            }
        }
    };

    Value::Value()
    {
    }
    Value::Value(const Value &v)
    {
        type = v.type;
        switch (v.type)
        {
        case ValueType::number:
            data_ptr = new double(*((double *)v.data_ptr));
            break;
        case ValueType::function:
        case ValueType::string:
            data_ptr = new String(*(String *)(v.data_ptr));
            break;
        case ValueType::boolean:
            data_ptr = new bool(*(bool *)(v.data_ptr));
            break;
        case ValueType::object:
            data_ptr = new Object(*(Object *)(v.data_ptr));
        case ValueType::array:
            data_ptr = new Array(*(Array *)(v.data_ptr));
        case ValueType::null:
        default:
            break;
        }
    }
    Value::Value(Value &&v)
    {
        std::swap(type, v.type);
        std::swap(data_ptr, v.data_ptr);
    }
    Value::Value(bool data)
    {
        type = ValueType::boolean;
        data_ptr = new bool(data);
    }
    Value::Value(const char *data)
    {
        type = ValueType::string;
        data_ptr = new String(data);
    }
    Value::Value(int data)
    {
        type = ValueType::number;
        data_ptr = new double(data);
    }
    Value::Value(std::nullptr_t null)
    {
        type = ValueType::null;
    }
    Value::Value(double data)
    {
        type = ValueType::number;
        data_ptr = new double(data);
    }
    Value::Value(std::string data)
    {
        type = ValueType::string;
        data_ptr = new String(data);
    }
    Value::Value(String data)
    {
        type = ValueType::string;
        data_ptr = new String(data);
    }

    Value::Value(Object obj)
    {
        type = ValueType::object;
        data_ptr = new Object(obj);
    }
    Value::Value(Array arr)
    {
        type = ValueType::array;
        data_ptr = new Array(arr);
    }
    ValueType Value::Type() const
    {
        return type;
    }

    Value &Value::operator=(const Value &v)
    {
        switch (v.type)
        {
        case ValueType::number:
            data_ptr = new double(*((double *)v.data_ptr));
            break;
        case ValueType::function:
        case ValueType::string:
            data_ptr = new String(*(String *)(v.data_ptr));
            break;
        case ValueType::boolean:
            data_ptr = new bool(*(bool *)(v.data_ptr)); // 传值
            break;
        case ValueType::object:
            data_ptr = new Object(*(Object *)(v.data_ptr));
        case ValueType::array:
            data_ptr = new Array(*(Array *)(v.data_ptr));
        case ValueType::null:
        default:
            break;
        }
        this->type = v.type;
        return *this;
    }


    String Value::TypeString() const
    {
        auto t = Type();
        return type_emun2str[static_cast<int>(t)];
    }

    Value &Value::operator[](String key)
    {
        CheckType(ValueType::object);
        return Obj()[key];
    }

    Object &Value::Obj()
    {
        CheckType(ValueType::object);
        return *(Object *)data_ptr;
    }

    const Object &Value::ObjC() const
    {
        CheckType(ValueType::object);
        return *(Object *)data_ptr;
    }

    Value &Value::operator[](int key)
    {
        CheckType(ValueType::array);
        return Arr()[key];
    }
    Array &Value::Arr()
    {
        CheckType(ValueType::array);
        return *(Array *)data_ptr;
    }

    const Array &Value::ArrC() const
    {
        CheckType(ValueType::array);
        return *(Array *)data_ptr;
    }

    bool Value::In(const String &key) const
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

    bool Value::NotUndef()
    {
        return type != ValueType::null;
    }

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
    double &Value::Number()
    {
        CheckType(ValueType::number);
        return *(double *)data_ptr;
    }
    bool &Value::Bool()
    {
        CheckType(ValueType::boolean);
        return *(bool *)data_ptr;
    }
    String &Value::Str()
    {
        CheckType(ValueType::string);
        return *(String *)data_ptr;
    }
    const double &Value::NumberC() const
    {
        CheckType(ValueType::number);
        return *(double *)data_ptr;
    }
    const bool &Value::BoolC() const
    {
        CheckType(ValueType::boolean);
        return *(bool *)data_ptr;
    }
    const String &Value::StrC() const
    {
        CheckType(ValueType::string);
        return *(String *)data_ptr;
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

    bool Value::DeepCompare(const Value &r) const
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
        if (type == ValueType::function)
        {
            return l.StrC() == r.StrC();
        }
        else if (type == ValueType::object)
        {

            if (l.ObjC().SrcC().size() != r.ObjC().SrcC().size())
            {
                return false;
            }

            for (auto &i : l.ObjC().SrcC())
            {
                if (!i.second.DeepCompare(r.ObjC().SrcC().at(i.first)))
                {
                    return false;
                }
            }
        }
        else if (type == ValueType::array)
        {

            auto &larr = l.ArrC().SrcC();
            auto &rarr = r.ArrC().SrcC();
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

    bool Value::operator==(const Value &rhs)
    {
        return DeepCompare(rhs);
    }
    Value Value::null = Value();
}
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
            case ValueType::undefined:
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
        case ValueType::undefined:
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
        case ValueType::undefined:
        default:
            break;
        }
        this->type = v.type;
        return *this;
    }

    Value &Value::operator[](String key)
    {
        return Obj()[key];
    }

    String Value::TypeString() const
    {
        auto t = Type();
        return jstype_emun2str[static_cast<int>(t)];
    }

    Object &Value::Obj()
    {
        if (type != ValueType::object)
        {
            THROW_MSG("get object了一个不是Object的Value实例");
        }
        return *(Object *)data_ptr;
    }

    const Object &Value::ObjC() const
    {
        if (type != ValueType::object)
        {
            THROW_MSG("get object了一个不是Object的Value实例");
        }
        return *(Object *)data_ptr;
    }

    Value &Value::operator[](int key)
    {
        return Arr()[key];
    }
    Array &Value::Arr()
    {
        if (type != ValueType::array)
        {
            THROW_MSG("get array了一个不是array的Value实例");
        }
        return *(Array *)data_ptr;
    }

    bool Value::In(const String &key) const
    {
        return this->ObjC().In(key);
    }
    bool Value::In(size_t idx) const
    {
        return this->ArrC().In(idx);
    }

    const Array &Value::ArrC() const
    {
        if (type != ValueType::array)
        {
            THROW_MSG("get array了一个不是array的Value实例");
        }
        return *(Array *)data_ptr;
    }
    bool Value::NotUndef()
    {
        return type != ValueType::undefined;
    }

    String Value::ToString() const
    {
        switch (type)
        {
        case ValueType::boolean:
            return GetC<bool>();
        case ValueType::number:
            return GetC<double>();
        case ValueType::string:
            return GetC<String>();
        case ValueType::null:
            return "null";
        case ValueType::undefined:
            return "undefined";
        case ValueType::array:
            return "[Array]";
        case ValueType::object:
            return "[Object]";
        case ValueType::function:
            return String::Format("[Function loc:{}]", GetC<String>());
        }
    }

    bool Value::ToBool() const
    {
        switch (type)
        {
        case ValueType::boolean:
            return GetC<bool>();
        case ValueType::number:
            return GetC<double>() != 0.0;
        case ValueType::string:
            return GetC<String>().size() != 0;
        case ValueType::null:
        case ValueType::undefined:
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
            return l.GetC<String>() == r.GetC<String>();
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
    Value Value::undefined = Value();
}
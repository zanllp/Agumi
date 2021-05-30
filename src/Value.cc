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
                delete (Array *)data_ptr;
                break;
            case JsType::null:
            case JsType::undefined:
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
            data_ptr = new Array(*(Array *)(v.data_ptr));
        case JsType::null:
        case JsType::undefined:
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
        type = JsType::boolean;
        data_ptr = new bool(data);
    }
    Value::Value(const char *data)
    {
        type = JsType::string;
        data_ptr = new String(data);
    }
    Value::Value(int data)
    {
        type = JsType::number;
        data_ptr = new double(data);
    }
    Value::Value(std::nullptr_t null)
    {
        type = JsType::null;
    }
    Value::Value(double data)
    {
        type = JsType::number;
        data_ptr = new double(data);
    }
    Value::Value(std::string data)
    {
        type = JsType::string;
        data_ptr = new String(data);
    }

    Value::Value(JsObject obj)
    {
        type = JsType::object;
        data_ptr = new JsObject(obj);
    }
    Value::Value(Array arr)
    {
        type = JsType::array;
        data_ptr = new Array(arr);
    }
    JsType Value::Type() const
    {
        return type;
    }

    Value &Value::operator=(const Value &v)
    {
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
            data_ptr = new Array(*(Array *)(v.data_ptr));
        case JsType::null:
        case JsType::undefined:
        default:
            break;
        }
        this->type = v.type;
        return *this;
    }

    Value &Value::operator[](String key)
    {
        return Object()[key];
    }

    String Value::TypeString() const
    {
        auto t = Type();
        return jstype_emun2str[static_cast<int>(t)];
    }

    JsObject &Value::Object()
    {
        if (type != JsType::object)
        {
            THROW_MSG("get object了一个不是Object的Value实例");
        }
        return *(JsObject *)data_ptr;
    }

    const JsObject &Value::ObjectC() const
    {
        if (type != JsType::object)
        {
            THROW_MSG("get object了一个不是Object的Value实例");
        }
        return *(JsObject *)data_ptr;
    }

    Value &Value::operator[](int key)
    {
        return Arr()[key];
    }
    Array &Value::Arr()
    {
        if (type != JsType::array)
        {
            THROW_MSG("get array了一个不是array的Value实例");
        }
        return *(Array *)data_ptr;
    }

    bool Value::In(const String &key) const
    {
        return this->ObjectC().In(key);
    }
    bool Value::In(size_t idx) const
    {
        return this->ArrC().In(idx);
    }

    const Array &Value::ArrC() const
    {
        if (type != JsType::array)
        {
            THROW_MSG("get array了一个不是array的Value实例");
        }
        return *(Array *)data_ptr;
    }
    bool Value::NotUndef()
    {
        return type != JsType::undefined;
    }

    String Value::ToString() const
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

    bool Value::ToBool() const
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
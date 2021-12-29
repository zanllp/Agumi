#pragma once
#include "stdafx.h"
#include "Object.h"
#include "Array.h"
#include "util.h"
namespace agumi
{
    
    class Value
    {
        friend Object;

    public:
        ~Value();

        Value();
        Value(const Value &v);
        Value(Value &&v);
        Value(bool data);
        Value(const char *data);
        Value(int data);
        Value(std::nullptr_t null);
        Value(double data);
        Value(std::string data);
        Value(Object obj);
        Value(Array arr);
        ValueType Type() const;
        template <typename T>
        constexpr T &Get()
        {
            CheckType(typeid(T));
            return *(T *)data_ptr;
        }

        template <typename T>
        constexpr T GetOr(T backup)
        {
            if (type == ValueType::null)
            {
                return backup;
            }
            
            CheckType(typeid(T));
            return *(T *)data_ptr;
        }

        template <typename T>
        constexpr const T &GetC() const
        {
            CheckType(typeid(T));
            return *(T *)data_ptr;
        }

        Value &operator=(const Value &v);
        Value &operator[](String key);
        Object &Obj();
        const Object &ObjC() const;
        Value &operator[](int key);
        Array &Arr();
        const Array &ArrC() const;
        String ToString() const;
        bool NotUndef();
        bool In(const String &key) const;
        bool In(size_t idx) const;
        bool DeepCompare(const Value &r) const;
        bool operator==(const Value &rhs);
        bool ToBool() const;
        String TypeString() const;

        static Value CreateFunc(String mem_key)
        {
            Value val = mem_key;
            val.type = ValueType::function;
            return val;
        }

        static Value null;

    private:
        ValueType type = ValueType::null;
        void *data_ptr = nullptr;

        constexpr void CheckType(const std::type_info &t) const
        {
            switch (type)
            {
            case ValueType::number:
                Assest(typeid(double).hash_code(), t.hash_code(), "类型不对");
                break;
            case ValueType::function:
            case ValueType::string:
                Assest(typeid(String).hash_code(), t.hash_code(), "类型不对");
                break;
            case ValueType::boolean:
                Assest(typeid(bool).hash_code(), t.hash_code(), "类型不对");
                break;
            case ValueType::null:
                Assest(typeid(nullptr).hash_code(), t.hash_code(), "类型不对");
                break;
            case ValueType::array:
            case ValueType::object:
                THROW_MSG("不应该使用get来获取Object/Array");
                break;
            }
        }
    };
}
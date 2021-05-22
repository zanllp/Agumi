#pragma once
#include "stdafx.h"
#include "JsObject.h"
#include "JsArray.h"
#include "util.h"
namespace agumi
{
    
    class JsValue
    {
        friend JsObject;

    public:
        ~JsValue();

        JsValue();
        JsValue(const JsValue &v);
        JsValue(JsValue &&v);
        JsValue(bool data);
        JsValue(const char *data);
        JsValue(int data);
        JsValue(std::nullptr_t null);
        JsValue(double data);
        JsValue(std::string data);
        JsValue(JsObject obj);
        JsValue(JsArray arr);
        JsType Type() const;
        template <typename T>
        constexpr T &Get()
        {
            CheckType(typeid(T));
            return *(T *)data_ptr;
        }

        template <typename T>
        constexpr const T &GetC() const
        {
            CheckType(typeid(T));
            return *(T *)data_ptr;
        }

        JsValue &operator=(const JsValue &v);
        JsValue &operator[](String key);
        JsObject &Object();
        const JsObject &ObjectC() const;
        JsValue &operator[](int key);
        JsArray &Array();
        const JsArray &ArrayC() const;
        String ToString() const;
        bool NotUndef();
        bool In(const String &key) const;
        bool In(size_t idx) const;
        bool DeepCompare(const JsValue &r) const;
        bool operator==(const JsValue &rhs);
        bool ToBool() const;
        String TypeString() const;

        static JsValue CreateFunc(String mem_key)
        {
            JsValue val = mem_key;
            val.type = JsType::function;
            return val;
        }

        static JsValue undefined;

    private:
        JsType type = JsType::undefined;
        void *data_ptr = nullptr;

        constexpr void CheckType(const std::type_info &t) const
        {
            switch (type)
            {
            case JsType::number:
                Assest(typeid(double).hash_code(), t.hash_code(), "类型不对");
                break;
            case JsType::function:
            case JsType::string:
                Assest(typeid(String).hash_code(), t.hash_code(), "类型不对");
                break;
            case JsType::boolean:
                Assest(typeid(bool).hash_code(), t.hash_code(), "类型不对");
                break;
            case JsType::null:
                Assest(typeid(nullptr).hash_code(), t.hash_code(), "类型不对");
                break;
            case JsType::undefined:
                THROW_MSG("get了一个未定义的JsValue实例");
                break;
            case JsType::array:
            case JsType::object:
                THROW_MSG("不应该使用get来获取jsObject/Array");
                break;
            }
        }
    };
}
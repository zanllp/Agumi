#pragma once
#include "Array.h"
#include "Object.h"
#include "stdafx.h"
#include "util.h"
namespace agumi
{

class Value
{
    friend Object;

  public:
    ~Value();

    Value();
    Value(const Value& v);
    Value(Value&& v);
    Value(bool data);
    Value(const char* data);
    Value(int data);
    Value(std::nullptr_t null);
    Value(double data);
    Value(std::string data);
    Value(String data);
    Value(Object obj);
    Value(Array arr);
    ValueType Type() const;

    template <typename T> constexpr T GetOr(T backup, ValueType expectType)
    {
        if (type == ValueType::null)
        {
            return backup;
        }
        CheckType(expectType);
        return std::get<T>(val);
    }
    double& Number();
    bool& Bool();
    String& Str();
    const double& NumberC() const;
    const bool& BoolC() const;
    const String& StrC() const;
    void CheckType(ValueType t) const;

    Value& operator=(const Value& v);
    Value& operator[](String key);
    Object& Obj();
    const Object& ObjC() const;
    Value& operator[](int key);
    Array& Arr();
    const Array& ArrC() const;
    String ToString() const;
    bool NotUndef() const;
    bool In(const String& key) const;
    bool In(size_t idx) const;
    bool DeepCompare(const Value& r) const;
    bool operator==(const Value& rhs);
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
    std::variant<bool, double, String, Object, Array> val;
    void ValCopy(const Value& v);
};
} // namespace agumi
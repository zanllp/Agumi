#include "JsonStringify.h"
#include "Object.h"
#include "Value.h"
#include "util.h"
namespace agumi
{
bool JsonStringify::error_if_circle_ref = false;
bool isPtrExist(std::set<long>& ptr_rec, const agumi::ObjectMap* ptr) { return ptr_rec.find(long(ptr)) != ptr_rec.end(); }
bool isPtrExist(std::set<long>& ptr_rec, const agumi::ArrayVec* ptr) { return ptr_rec.find(long(ptr)) != ptr_rec.end(); }
String JsonStringify::StringifyInternalArray(const Array& next, std::set<long> ptr_rec, int indent_step, int indent, bool escape)
{
    ptr_rec.insert(long(next.Ptr()));
    Vector<String> res_vec;
    auto handle_circle_ref = [&]
    {
        if (error_if_circle_ref)
        {
            THROW_MSG("循环引用");
        }
        res_vec.push_back("\"circle ref\"");
    };
    int next_indent = indent + indent_step;
    for (auto& i : next.SrcC())
    {
        auto type = i.Type();
        if (type == ValueType::object)
        {
            if (!isPtrExist(ptr_rec, i.ObjC().Ptr()))
            {
                auto next_v = JsonStringify::StringifyInternal(i.ObjC(), ptr_rec, indent_step, next_indent, escape);
                res_vec.push_back(next_v);
            }
            else
            {
                handle_circle_ref();
            }
        }
        else if (type == ValueType::array)
        {
            if (!isPtrExist(ptr_rec, i.ArrC().Ptr()))
            {
                auto next_v = JsonStringify::StringifyInternalArray(i.ArrC(), ptr_rec, indent_step, next_indent, escape);
                res_vec.push_back(next_v);
            }
            else
            {
                handle_circle_ref();
            }
        }
        else
        {
            auto v_src = i.ToString();
            auto str = escape ? v_src.Escape() : v_src;
            auto value_str = i.Type() == ValueType::string ? String::Format("\"{}\"", str) : str;
            res_vec.push_back(value_str);
        }
    }
    return SameLevelCompisition(res_vec, indent_step, next_indent, {"[", "]"});
}
String JsonStringify::StringifyInternal(const Object& next, std::set<long> ptr_rec, int indent_step, int indent, bool escape)
{
    ptr_rec.insert(long(next.Ptr()));
    std::vector<String> res_vec;
    auto handle_circle_ref = [&](String key)
    {
        if (error_if_circle_ref)
        {
            THROW_MSG("key:{} 循环引用", key);
        }
        res_vec.push_back(String::Format("\"{}\": \"circle ref\"", key));
    };
    int next_indent = indent + indent_step;
    for (auto& i : next.SrcC())
    {
        auto key = escape ? i.first.Escape() : i.first;
        auto type = i.second.Type();
        if (type == ValueType::object)
        {
            if (!isPtrExist(ptr_rec, i.second.ObjC().Ptr()))
            {
                auto next_v = JsonStringify::StringifyInternal(i.second.ObjC(), ptr_rec, indent_step, next_indent, escape);
                auto str = String::Format("\"{}\": {}", key, next_v);
                res_vec.push_back(str);
            }
            else
            {
                handle_circle_ref(key);
            }
        }
        else if (type == ValueType::array)
        {
            if (!isPtrExist(ptr_rec, i.second.ArrC().Ptr()))
            {
                auto next_v = JsonStringify::StringifyInternalArray(i.second.ArrC(), ptr_rec, indent_step, next_indent, escape);
                auto str = String::Format("\"{}\": {}", key, next_v);
                res_vec.push_back(str);
            }
            else
            {
                handle_circle_ref(key);
            }
        }
        else
        {
            String v_src = i.second.ToString();
            auto str = escape ? v_src.Escape() : v_src;
            auto value_str = (i.second.Type() == ValueType::string || i.second.Type() == ValueType::function)
                                 ? String::Format("\"{}\"", str.Replace('\n', "\\n", -1))
                                 : str;
            res_vec.push_back(String::Format("\"{}\": {}", key, value_str));
        }
    }
    return SameLevelCompisition(res_vec, indent_step, next_indent, {"{", "}"});
}

String JsonStringify::SameLevelCompisition(std::vector<String>& src_vec, int indent_step, int indent, std::tuple<String, String> start_end_symbol)
{
    bool zero_indent = indent_step == 0;
    bool is_empty = src_vec.size() == 0;
    String cr_if_not_zero_indent = (zero_indent || is_empty) ? "" : "\n";
    std::stringstream res;
    res << std::get<0>(start_end_symbol) << cr_if_not_zero_indent;
    int vec_size = src_vec.size();
    const String space = " ";
    for (size_t i = 0; i < vec_size; i++)
    {
        res << (zero_indent ? "" : space.Repeat(indent));
        res << src_vec[i];
        if (i != src_vec.size() - 1)
        {
            res << ",";
        }
        res << cr_if_not_zero_indent;
    }
    res << ((zero_indent || is_empty) ? "" : space.Repeat(indent - indent_step));
    res << std::get<1>(start_end_symbol);
    return res.str();
}

String JsonStringify::Call(const Value& v, int indent, bool escape)
{
    std::set<long> ptr_rec;
    switch (v.Type())
    {
    case ValueType::object:
        return JsonStringify::StringifyInternal(v.ObjC(), ptr_rec, indent, 0, escape);
    case ValueType::array:
        return JsonStringify::StringifyInternalArray(v.ArrC(), ptr_rec, indent, 0, escape);
    case ValueType::string:
    case ValueType::function:
        return String::Format("\"{}\"", v.ToString());
    default:
        return v.ToString();
    }
}
} // namespace agumi

#include "Json.h"
#include "Value.h"
#include "JsObject.h"
#include "util.h"
namespace agumi
{
    bool Json::error_if_circle_ref = false;
    String Json::StringifyInternalArray(const Array &next,
                                        std::set<const JsObjectMap *> &json_obj_rec, std::set<const ArrayVec *> &json_arr_rec,
                                        int indent_step, int indent, bool escape)
    {
        json_arr_rec.insert(next.Ptr());
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
        for (auto &i : next.SrcC())
        {
            auto type = i.Type();
            if (type == JsType::object)
            {
                if (json_obj_rec.find(i.ObjectC().Ptr()) == json_obj_rec.end())
                {
                    auto next_v = Json::StringifyInternal(i.ObjectC(), json_obj_rec, json_arr_rec, indent_step, next_indent, escape);
                    res_vec.push_back(next_v);
                }
                else
                {
                    handle_circle_ref();
                }
            }
            else if (type == JsType::array)
            {
                if (json_arr_rec.find(i.ArrC().Ptr()) == json_arr_rec.end())
                {
                    auto next_v = Json::StringifyInternalArray(i.ArrC(), json_obj_rec, json_arr_rec, indent_step, next_indent, escape);
                    res_vec.push_back(next_v);
                }
                else
                {
                    handle_circle_ref();
                }
            }
            else
            {
                auto v_src = type == JsType::undefined ? "null" : i.ToString();
                auto str = escape ? v_src.Escape() : v_src;
                auto value_str = i.Type() == JsType::string ? String::Format("\"{}\"", str) : str;
                res_vec.push_back(value_str);
            }
        }
        return SameLevelCompisition(res_vec, indent_step, next_indent, {"[", "]"});
    }
    String Json::StringifyInternal(const JsObject &next,
                                   std::set<const JsObjectMap *> &json_obj_rec, std::set<const ArrayVec *> &json_arr_rec,
                                   int indent_step, int indent, bool escape)
    {
        json_obj_rec.insert(next.Ptr());
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
        for (auto &i : next.SrcC())
        {
            auto key = escape ? i.first.Escape() : i.first;
            auto type = i.second.Type();
            if (type == JsType::object)
            {
                if (json_obj_rec.find(i.second.ObjectC().Ptr()) == json_obj_rec.end())
                {
                    auto next_v = Json::StringifyInternal(i.second.ObjectC(), json_obj_rec, json_arr_rec, indent_step, next_indent, escape);
                    auto str = String::Format("\"{}\": {}", key, next_v);
                    res_vec.push_back(str);
                }
                else
                {
                    handle_circle_ref(key);
                }
            }
            else if (type == JsType::array)
            {
                if (json_arr_rec.find(i.second.ArrC().Ptr()) == json_arr_rec.end())
                {
                    auto next_v = Json::StringifyInternalArray(i.second.ArrC(), json_obj_rec, json_arr_rec, indent_step, next_indent, escape);
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
                String v_src = (type == JsType::undefined) ? "null" : i.second.ToString();
                auto str = escape ? v_src.Escape() : v_src;
                auto value_str = i.second.Type() == JsType::string ? String::Format("\"{}\"", str.Replace('\n', "\\n", -1)) : str;
                res_vec.push_back(String::Format("\"{}\": {}", key, value_str));
            }
        }
        return SameLevelCompisition(res_vec, indent_step, next_indent, {"{", "}"});
    }

    String Json::SameLevelCompisition(std::vector<String> &src_vec, int indent_step, int indent, std::tuple<String, String> start_end_symbol)
    {
        bool zero_indent = indent_step == 0;
        bool is_empty = src_vec.size() == 0;
        String cr_if_not_zero_indent = (zero_indent || is_empty) ? "" : "\n";
        std::stringstream res;
        res << std::get<0>(start_end_symbol) << cr_if_not_zero_indent;
        int vec_size = src_vec.size();
        for (size_t i = 0; i < vec_size; i++)
        {
            res << (zero_indent ? "" : String(" ").Repeat(indent));
            res << src_vec[i];
            if (i != src_vec.size() - 1)
            {
                res << ",";
            }
            res << cr_if_not_zero_indent;
        }
        res << ((zero_indent || is_empty) ? "" : String(" ").Repeat(indent - indent_step));
        res << std::get<1>(start_end_symbol);
        return res.str();
    }

    String Json::Stringify(const Value &v, int indent, bool escape)
    {
        std::set<const JsObjectMap *> json_obj_rec;
        std::set<const ArrayVec *> json_arr_rec;
        switch (v.Type())
        {
        case JsType::object:
            return Json::StringifyInternal(v.ObjectC(), json_obj_rec, json_arr_rec, indent, 0, escape);
        case JsType::array:
            return Json::StringifyInternalArray(v.ArrC(), json_obj_rec, json_arr_rec, indent, 0, escape);
        case JsType::string:
            return String::Format("\"{}\"", v.ToString());
        default:
            return v.ToString();
        }
    }
}